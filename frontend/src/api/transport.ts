import { REQUEST_TIMEOUT_MS } from '../constants/ros.ts';
import { ApiError } from '@/types/errors';
import type { TransportExecutor } from './core.ts';

export function createFetchExecutor(): TransportExecutor {
  return async (ctx) => {
    let response: Response;
    const timeoutSignal = AbortSignal.timeout(
      ctx.request.timeoutMs ?? REQUEST_TIMEOUT_MS,
    );
    const signal =
      ctx.request.signal !== undefined
        ? AbortSignal.any([ctx.request.signal, timeoutSignal])
        : timeoutSignal;
    try {
      response = await fetch(ctx.url, {
        method: ctx.request.method ?? 'GET',
        signal,
        ...(ctx.request.body != null && {
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(ctx.request.body),
        }),
      });
    } catch (error) {
      const name = error instanceof Error ? error.name : '';
      const timedOut = name === 'TimeoutError';
      const aborted = timedOut || name === 'AbortError';
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
