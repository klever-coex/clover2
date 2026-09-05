import { REQUEST_TIMEOUT_MS } from '../constants/ros.ts';
import { ApiError } from '@/types/errors';
import type { TransportExecutor } from './core.ts';

export function createFetchExecutor(): TransportExecutor {
  return async (ctx) => {
    let response: Response;
    try {
      response = await fetch(ctx.url, {
        method: ctx.request.method ?? 'GET',
        signal: AbortSignal.timeout(ctx.request.timeoutMs ?? REQUEST_TIMEOUT_MS),
        ...(ctx.request.body !== undefined && {
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(ctx.request.body),
        }),
      });
    } catch (error) {
      const aborted =
        error instanceof Error &&
        (error.name === 'TimeoutError' || error.name === 'AbortError');
      throw new ApiError(
        aborted ? `Request to ${ctx.url} timed out` : `Cannot reach ${ctx.url}`,
        0,
        aborted ? 'timeout' : 'network',
        { cause: error },
      );
    }

    if (!response.ok) {
      throw new ApiError(await readErrorMessage(response), response.status);
    }

    ctx.response = response;
  };
}

async function readErrorMessage(response: Response): Promise<string> {
  try {
    const body = (await response.json()) as { error?: unknown; error_message?: unknown };
    if (typeof body.error === 'string') return body.error;
    if (typeof body.error_message === 'string') return body.error_message;
  } catch {
  }
  return `HTTP ${response.status}`;
}
