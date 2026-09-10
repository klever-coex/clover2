import { REQUEST_TIMEOUT_MS } from '../constants/ros.ts';
import { ApiError } from '@/types/errors';
import type { Capability } from '@/types/manifest';

export interface ApiCallOptions {
  capabilities?: readonly Capability[];
  method?: 'GET' | 'POST' | 'PUT' | 'DELETE';
  body?: unknown;
  timeoutMs?: number;
  signal?: AbortSignal;
}

export type HttpCall = <T>(path: string, options?: ApiCallOptions) => Promise<T>;

export type CapabilityGate = (capability: Capability) => Promise<void>;

export function createHttpCall(httpBase: string, gate?: CapabilityGate): HttpCall {
  return async <T>(path: string, options?: ApiCallOptions): Promise<T> => {
    const url = httpBase + path;

    if (gate !== undefined) {
      for (const capability of options?.capabilities ?? []) {
        await gate(capability);
      }
    }

    const timeoutSignal = AbortSignal.timeout(
      options?.timeoutMs ?? REQUEST_TIMEOUT_MS,
    );
    const signal =
      options?.signal !== undefined
        ? AbortSignal.any([options.signal, timeoutSignal])
        : timeoutSignal;

    let response: Response;
    try {
      response = await fetch(url, {
        method: options?.method ?? 'GET',
        signal,
        ...(options?.body != null && {
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(options.body),
        }),
      });
    } catch (error) {
      const name = error instanceof Error ? error.name : '';
      const timedOut = name === 'TimeoutError';
      const aborted = timedOut || name === 'AbortError';
      throw new ApiError(
        aborted ? `Request to ${url} timed out` : `Cannot reach ${url}`,
        0,
        aborted ? 'timeout' : 'network',
        { cause: error },
      );
    }

    if (!response.ok) {
      throw new ApiError(await readErrorMessage(response), response.status);
    }

    const text = await response.text();
    if (text === '') return null as T;
    try {
      return JSON.parse(text) as T;
    } catch {
      throw new ApiError(`Invalid JSON response from ${path}`, response.status);
    }
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
