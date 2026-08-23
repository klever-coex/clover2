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
