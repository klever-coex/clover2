import type { HttpMiddleware } from '../core.ts';

export const baseUrlMiddleware: HttpMiddleware = (ctx, next) => {
  ctx.url = ctx.httpBase + ctx.request.path;
  return next();
};
