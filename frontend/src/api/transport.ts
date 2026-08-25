import { ApiError } from '../types/errors.ts';
import type { TransportExecutor } from './core.ts';

export function createFetchExecutor(): TransportExecutor {
  return async (ctx) => {
    const response = await fetch(ctx.url, {
      method: ctx.request.method ?? 'GET',
      ...(ctx.request.body !== undefined && {
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(ctx.request.body),
      }),
    });

    if (!response.ok) {
      throw new ApiError(await readErrorMessage(response), response.status);
    }

    ctx.response = response;
  };
}

async function readErrorMessage(response: Response): Promise<string> {
  try {
    const body = (await response.json()) as { error?: unknown };
    if (typeof body.error === 'string') return body.error;
  } catch {
  }
  return `HTTP ${response.status}`;
}
