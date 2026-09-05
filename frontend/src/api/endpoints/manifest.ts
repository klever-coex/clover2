import { ApiError } from '@/types/errors';
import type { Capability, Manifest } from '@/types/manifest';
import type { HttpCall } from '../core.ts';

/** The manifest is refetched after this long so backend restarts are picked up. */
const MANIFEST_TTL_MS = 30_000;

export interface ManifestEndpoints {
  get(): Promise<Manifest>;
  clearCache(): void;
  requireCapability(capability: Capability): Promise<void>;
}

export function createManifestEndpoints(http: HttpCall): ManifestEndpoints {
  let manifestPromise: Promise<Manifest> | null = null;
  let fetchedAt = Number.NEGATIVE_INFINITY;
  const ensureManifest = (): Promise<Manifest> => {
    if (manifestPromise !== null && Date.now() - fetchedAt < MANIFEST_TTL_MS) {
      return manifestPromise;
    }

    fetchedAt = Date.now();
    manifestPromise = http<Manifest>('/api/manifest').catch((error: unknown) => {
      manifestPromise = null;
      throw error;
    });
    return manifestPromise;
  };

  return {
    get: ensureManifest,

    clearCache: () => {
      manifestPromise = null;
    },

    requireCapability: async (capability) => {
      const manifest = await ensureManifest();
      const supported = manifest.plugins.some((plugin) =>
        plugin.capabilities.includes(capability),
      );
      if (!supported) {
        throw new ApiError(`Missing backend capability: ${capability}`);
      }
    },
  };
}
