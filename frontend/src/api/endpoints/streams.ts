import { ApiError, isErrorFrame, toApiError } from '../../types/errors.ts';
import type { Capability } from '../../types/manifest.ts';
import type { RosJsonValue } from '../../types/stream.ts';
import { encodeRosPath } from '../url.ts';

export interface TopicSubscriptionOptions {
  onMessage: (message: RosJsonValue) => void;
  onError?: (error: ApiError) => void;
  onClose?: (info: { code: number; reason: string }) => void;
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

      const fail = (error: ApiError) => {
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
            if (disposed) return;
            disposed = true;

            if (ERROR_CLOSE_CODES.has(event.code)) {
              const reason = event.reason ? ` ${event.reason}` : '';
              options.onError?.(
                new ApiError(`WebSocket closed (${event.code})${reason}`, event.code),
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
          if (disposed) return;
          disposed = true;
          socket?.close();
        },
      };
    },
  };
}
