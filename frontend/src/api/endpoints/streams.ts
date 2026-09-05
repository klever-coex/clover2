import { WS_CONNECT_TIMEOUT_MS } from '../../constants/ros.ts';
import { ApiError, isErrorFrame, toApiError } from '@/types/errors';
import type { Capability } from '@/types/manifest';
import type { RosJsonValue } from '@/types/stream';
import { encodeRosPath } from '../url.ts';

export interface TopicSubscriptionOptions {
  onMessage: (message: RosJsonValue) => void;
  onError?: (error: ApiError) => void;
  onClose?: (info: { code: number; reason: string }) => void;
  /**
   * The backend closes idle connections after 60 s even while streaming;
   * periodically sending a text frame (ignored by the server) keeps it alive.
   */
  keepaliveIntervalMs?: number;
}

export interface TopicSubscription {
  close: () => void;
}

const ERROR_CLOSE_CODES = new Set([1006, 1008, 1011]);

export interface StreamsEndpoints {
  subscribe(
    topicName: string,
    options: TopicSubscriptionOptions,
  ): TopicSubscription;
}

export function createStreamsEndpoints(
  wsBase: string,
  requireCapability: (capability: Capability) => Promise<void>,
): StreamsEndpoints {
  return {
    subscribe(topicName, options) {
      let socket: WebSocket | null = null;
      let disposed = false;
      let connectTimer: ReturnType<typeof setTimeout> | null = null;
      let keepaliveTimer: ReturnType<typeof setInterval> | null = null;

      const clearConnectTimer = () => {
        if (connectTimer === null) return;
        clearTimeout(connectTimer);
        connectTimer = null;
      };

      const clearKeepaliveTimer = () => {
        if (keepaliveTimer === null) return;
        clearInterval(keepaliveTimer);
        keepaliveTimer = null;
      };

      const fail = (error: ApiError) => {
        clearConnectTimer();
        clearKeepaliveTimer();
        if (disposed) return;
        disposed = true;
        options.onError?.(error);
      };

      void requireCapability('topics')
        .then(() => {
          if (disposed) return;

          socket = new WebSocket(
            `${wsBase}/ws/topic/json/-/${encodeRosPath(topicName)}`,
          );

          // A handshake that stalls (host reachable, nothing answering) fires
          // neither onclose nor onerror, which used to leave the page connecting
          // forever.
          const pending = socket;
          connectTimer = setTimeout(() => {
            connectTimer = null;
            fail(new ApiError(`WebSocket connect to ${topicName} timed out`, 0, 'network'));
            pending.close();
          }, WS_CONNECT_TIMEOUT_MS);

          socket.onopen = () => {
            clearConnectTimer();
            if (options.keepaliveIntervalMs !== undefined) {
              keepaliveTimer = setInterval(() => {
                if (socket?.readyState === WebSocket.OPEN) {
                  socket.send('ping');
                }
              }, options.keepaliveIntervalMs);
            }
          };

          socket.onerror = () => {
            fail(new ApiError(`WebSocket error on ${topicName}`, 0, 'network'));
          };

          socket.onmessage = (event) => {
            if (disposed) return;

            let parsed: unknown;
            try {
              parsed = JSON.parse(String(event.data));
            } catch {
              console.warn('clover2: ignoring non-JSON WebSocket frame');
              return;
            }

            if (isErrorFrame(parsed)) {
              fail(new ApiError(parsed.error));
              return;
            }

            options.onMessage(parsed as RosJsonValue);
          };

          socket.onclose = (event) => {
            clearConnectTimer();
            clearKeepaliveTimer();
            if (disposed) return;
            disposed = true;

            if (ERROR_CLOSE_CODES.has(event.code)) {
              const reason = event.reason ? ` ${event.reason}` : '';
              options.onError?.(
                new ApiError(
                  `WebSocket closed (${event.code})${reason}`,
                  event.code,
                  event.code === 1006 ? 'network' : 'backend',
                ),
              );
            } else {
              options.onClose?.({ code: event.code, reason: event.reason });
            }
          };
        })
        .catch((error: unknown) => {
          fail(toApiError(error));
        });

      return {
        close: () => {
          clearConnectTimer();
          clearKeepaliveTimer();
          if (disposed) return;
          disposed = true;
          socket?.close();
        },
      };
    },
  };
}
