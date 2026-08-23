import { ApiError } from '../../types/errors.ts';
import type { HttpMiddleware } from '../core.ts';

export const jsonMiddleware: HttpMiddleware = async (ctx, next) => {
  await next();

  if (ctx.response === null) return;

  try {
    ctx.body = await ctx.response.json();
  } catch {
    throw new ApiError(
      `Invalid JSON response from ${ctx.request.path}`,
      ctx.response.status,
    );
  }
};
