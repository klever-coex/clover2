import { ApiError } from '@/types/errors';
import type { HttpMiddleware } from '../core.ts';

export const jsonMiddleware: HttpMiddleware = async (ctx, next) => {
  await next();

  if (ctx.response === null) return;

  const text = await ctx.response.text();
  if (text === '') {
    ctx.body = null;
    return;
  }

  try {
    ctx.body = JSON.parse(text);
  } catch {
    throw new ApiError(
      `Invalid JSON response from ${ctx.request.path}`,
      ctx.response.status,
    );
  }
};
