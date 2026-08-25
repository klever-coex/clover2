import type { Capability } from '../types/manifest.ts';

export interface ApiRequest {
  path: string;
  capabilities?: readonly Capability[];
  /** HTTP method; GET when omitted. */
  method?: 'GET' | 'POST' | 'PUT' | 'DELETE';
  /** JSON request body; only meaningful for non-GET methods. */
  body?: unknown;
}

export interface ApiContext {
  readonly request: ApiRequest;
  readonly httpBase: string;

  url: string;
  response: Response | null;
  body: unknown;
}

export type HttpMiddleware = (
  ctx: ApiContext,
  next: () => Promise<void>,
) => Promise<void>;

export type TransportExecutor = (ctx: ApiContext) => Promise<void>;

export interface HttpCall {
  <T>(
    path: string,
    options?: {
      capabilities?: readonly Capability[];
      method?: 'GET' | 'POST' | 'PUT' | 'DELETE';
      body?: unknown;
    },
  ): Promise<T>;
}

export function createHttpCall(
  httpBase: string,
  middlewares: readonly HttpMiddleware[],
  executor: TransportExecutor,
): HttpCall {
  return async <T>(
    path: string,
    options?: {
      capabilities?: readonly Capability[];
      method?: 'GET' | 'POST' | 'PUT' | 'DELETE';
      body?: unknown;
    },
  ): Promise<T> => {
    const ctx: ApiContext = {
      request: {
        path,
        capabilities: options?.capabilities,
        method: options?.method,
        body: options?.body,
      },
      httpBase,
      url: '',
      response: null,
      body: null,
    };

    const dispatch = (index: number): Promise<void> => {
      const middleware = middlewares[index];
      if (middleware === undefined) {
        return executor(ctx);
      }
      return middleware(ctx, () => dispatch(index + 1));
    };

    await dispatch(0);
    return ctx.body as T;
  };
}
