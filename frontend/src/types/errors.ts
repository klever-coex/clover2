export type ApiErrorKind = 'backend' | 'network' | 'timeout';

export class ApiError extends Error {
  readonly status: number;
  readonly kind: ApiErrorKind;

  constructor(
    message: string,
    status = 0,
    kind: ApiErrorKind = 'backend',
    options?: ErrorOptions,
  ) {
    super(message, options);
    this.name = 'ApiError';
    this.status = status;
    this.kind = kind;
  }
}

export function toApiError(error: unknown): ApiError {
  if (error instanceof ApiError) return error;
  return new ApiError(error instanceof Error ? error.message : String(error), 0);
}

export function isErrorFrame(value: unknown): value is { error: string } {
  return (
    typeof value === 'object' &&
    value !== null &&
    typeof (value as Record<string, unknown>).error === 'string'
  );
}
