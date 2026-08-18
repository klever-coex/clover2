// Wire types of the clover2_http backend (default port 8080).

export type Capability = 'nodes' | 'topics' | (string & {});

export interface PluginManifest {
  name: string;
  version: number;
  capabilities: Capability[];
}

export interface Manifest {
  plugins: PluginManifest[];
}

export interface TopicInfo {
  name: string;
  type: string;
}

export interface NodeInfo {
  name: string;
}

export interface NodeDetail {
  name: string;
  ns: string;
  publishers: TopicInfo[];
  subscribers: TopicInfo[];
}

/** One ROS message deserialized to JSON. rclcpp serialization is acyclic, so no recursion guard is needed. */
export type RosJsonValue =
  | null
  | boolean
  | number
  | string
  | RosJsonValue[]
  | { [field: string]: RosJsonValue };

/** Error raised by the clover2_http backend; `status` is 0 for network/WebSocket failures. */
export class ApiError extends Error {
  readonly status: number;

  constructor(message: string, status = 0) {
    super(message);
    this.name = 'ApiError';
    this.status = status;
  }
}

/** Normalizes an unknown thrown value into an ApiError. */
export function toApiError(error: unknown): ApiError {
  if (error instanceof ApiError) return error;
  return new ApiError(error instanceof Error ? error.message : String(error), 0);
}

/** Backend error payload shape: {"error": "..."} */
export function isErrorFrame(value: unknown): value is { error: string } {
  return (
    typeof value === 'object' &&
    value !== null &&
    typeof (value as Record<string, unknown>).error === 'string'
  );
}
