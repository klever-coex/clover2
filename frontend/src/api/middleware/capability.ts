import type { Capability } from '../../types/manifest.ts';
import type { HttpMiddleware } from '../core.ts';

export type CapabilityGate = (capability: Capability) => Promise<void>;

export function capabilityMiddleware(gate: CapabilityGate): HttpMiddleware {
  return async (ctx, next) => {
    for (const capability of ctx.request.capabilities ?? []) {
      await gate(capability);
    }
    await next();
  };
}
