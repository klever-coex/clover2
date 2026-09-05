import { toApiError } from '@/types/errors';
import type { HttpMiddleware } from '../core.ts';

export const errorsMiddleware: HttpMiddleware = async (_ctx, next) => {
  try {
    await next();
  } catch (error) {
    throw toApiError(error);
  }
};
