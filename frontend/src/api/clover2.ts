import { HTTP_PORT } from '../constants/ros.ts';
import { ApiError, isErrorFrame, toApiError } from '../types/ros.ts';
import type { Capability, Manifest, NodeDetail, RosJsonValue, TopicInfo } from '../types/ros.ts';

export interface TopicSubscriptionOptions {
  onMessage: (message: RosJsonValue) => void;
  onError?: (error: ApiError) => void;
  onClose?: (info: { code: number; reason: string }) => void;
}

export interface TopicSubscription {
  /** Idempotent; suppresses the socket's trailing close event. */
  close: () => void;
}

export interface Clover2Api {
  getManifest: () => Promise<Manifest>;
  getTopics: () => Promise<TopicInfo[]>;
  getNodes: () => Promise<string[]>;
  getNodeInfo: (nodeName: string) => Promise<NodeDetail>;
  subscribeTopic: (topicName: string, options: TopicSubscriptionOptions) => TopicSubscription;
  /** Drops the cached manifest so the next getManifest() re-fetches (e.g. after a backend restart). */
  clearManifestCache: () => void;
}

/** Close codes that mean the stream failed (vs. a normal server shutdown, 1000). */
const ERROR_CLOSE_CODES = new Set([1006, 1008, 1011]);

/** Converts "http://..." to "ws://..." and "https://..." to "wss://...". */
function toWebSocketBase(baseUrl: string): string {
  return baseUrl.replace(/^http/, 'ws');
}

/** Resolves the backend base URL from the current page: same host, HTTP_PORT. */
function resolveBaseUrl(): string {
  const { protocol, hostname } = window.location;
  return `${protocol}//${hostname}:${HTTP_PORT}`;
}

/** Encodes a ROS name for a path parameter: "/drone/tf" → "drone/tf". Each segment is encoded separately. */
function encodeRosPath(name: string): string {
  return name
    .replace(/^\/+/, '')
    .split('/')
    .map(encodeURIComponent)
    .join('/');
}

/**
 * Manifest-aware client for the clover2_http backend.
 *
 * The backend advertises plugin capabilities via GET /manifest; every feature
 * is gated on a capability and calls without backend support fail with ApiError.
 */
class Clover2HttpClient implements Clover2Api {
  private readonly httpBase: string;
  private readonly wsBase: string;
  private manifestPromise: Promise<Manifest> | null = null;

  constructor(baseUrl?: string) {
    const base = baseUrl ?? resolveBaseUrl();
    this.httpBase = base;
    this.wsBase = toWebSocketBase(base);
  }

  getManifest(): Promise<Manifest> {
    // Single-flight: parallel callers (e.g. StrictMode double effects) share one request.
    this.manifestPromise ??= this.getJson<Manifest>('/manifest');
    return this.manifestPromise;
  }

  clearManifestCache(): void {
    this.manifestPromise = null;
  }

  async getTopics(): Promise<TopicInfo[]> {
    await this.requireCapability('topics');
    const response = await this.getJson<{ topics: TopicInfo[] }>('/topics');
    return response.topics;
  }

  async getNodes(): Promise<string[]> {
    await this.requireCapability('nodes');
    const response = await this.getJson<{ nodes: string[] }>('/nodes');
    return response.nodes;
  }

  async getNodeInfo(nodeName: string): Promise<NodeDetail> {
    await this.requireCapability('nodes');
    return this.getJson<NodeDetail>(`/node/info/-/${encodeRosPath(nodeName)}`);
  }

  subscribeTopic(topicName: string, options: TopicSubscriptionOptions): TopicSubscription {
    let socket: WebSocket | null = null;
    let disposed = false;

    const fail = (error: ApiError) => {
      if (disposed) return;
      disposed = true;
      options.onError?.(error);
    };

    // Capability is checked asynchronously; the WebSocket opens only when supported.
    void this.requireCapability('topics')
      .then(() => {
        if (disposed) return;

        socket = new WebSocket(`${this.wsBase}/topic/json/-/${encodeRosPath(topicName)}`);

        socket.onmessage = (event) => {
          if (disposed) return;

          let parsed: unknown;
          try {
            parsed = JSON.parse(String(event.data));
          } catch {
            console.warn('clover2: ignoring non-JSON WebSocket frame');
            return;
          }

          // A frame of shape {"error": "..."} is a backend error, not a topic message.
          // Trade-off: a topic whose root message field is literally named "error" is misread.
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
            // 1008 = unknown topic, 1011 = server error, 1006 = abnormal/network loss.
            const reason = event.reason ? ` ${event.reason}` : '';
            options.onError?.(new ApiError(`WebSocket closed (${event.code})${reason}`, event.code));
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
  }

  /** Ensures the backend advertises `capability`; throws ApiError otherwise. */
  private async requireCapability(capability: Capability): Promise<void> {
    const manifest = await this.getManifest();
    const supported = manifest.plugins.some((plugin) => plugin.capabilities.includes(capability));
    if (!supported) {
      throw new ApiError(`Missing backend capability: ${capability}`);
    }
  }

  private async getJson<T>(path: string): Promise<T> {
    let response: Response;
    try {
      response = await fetch(`${this.httpBase}${path}`);
    } catch (error) {
      throw toApiError(error);
    }

    if (!response.ok) {
      throw new ApiError(await this.readErrorMessage(response), response.status);
    }
    return (await response.json()) as T;
  }

  /** Backend errors carry {"error": "..."} bodies; falls back to a status message. */
  private async readErrorMessage(response: Response): Promise<string> {
    try {
      const body = (await response.json()) as { error?: unknown };
      if (typeof body.error === 'string') return body.error;
    } catch {
      // Non-JSON error body; fall through.
    }
    return `HTTP ${response.status}`;
  }
}

export function createClover2Api(baseUrl?: string): Clover2Api {
  return new Clover2HttpClient(baseUrl);
}

/** Shared client instance bound to the current page origin. */
export const clover2Api: Clover2Api = createClover2Api();
