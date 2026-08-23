import { ApiError } from '../../types/errors.ts';
import type { Capability, Manifest } from '../../types/manifest.ts';
import type { HttpCall } from '../core.ts';

export interface ManifestEndpoints {
  get(): Promise<Manifest>;
  clearCache(): void;
  requireCapability(capability: Capability): Promise<void>;
}

export function createManifestEndpoints(http: HttpCall): ManifestEndpoints {
  let manifestPromise: Promise<Manifest> | null = null;

  return {
    get: () => {
      manifestPromise ??= http<Manifest>('/api/manifest');
      return manifestPromise;
    },

    clearCache: () => {
      manifestPromise = null;
    },

    requireCapability: async (capability) => {
      const manifest = await (manifestPromise ??= http<Manifest>('/api/manifest'));
      const supported = manifest.plugins.some((plugin) =>
        plugin.capabilities.includes(capability),
      );
      if (!supported) {
        throw new ApiError(`Missing backend capability: ${capability}`);
      }
    },
  };
}
