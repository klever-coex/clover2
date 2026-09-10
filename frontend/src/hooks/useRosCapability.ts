import { useEffect } from 'react';

import { useRosStore } from '@/store/useRosStore';
import type { ApiError } from '@/types/errors';
import type { Capability } from '@/types/manifest';

export interface RosCapability {
  ready: boolean;
  allowed: boolean;
  error: ApiError | null;
  retry: () => void;
}

export function useRosCapability(capability: Capability): RosCapability {
  const manifest = useRosStore((s) => s.manifest);
  const manifestLoading = useRosStore((s) => s.manifestLoading);
  const manifestError = useRosStore((s) => s.manifestError);
  const reloadManifest = useRosStore((s) => s.reloadManifest);
  const hasCapability = useRosStore((s) => s.hasCapability);

  useEffect(() => {
    if (manifest === null && !manifestLoading && manifestError === null) {
      void reloadManifest();
    }
  }, [manifest, manifestLoading, manifestError, reloadManifest]);

  return {
    ready: manifest !== null,
    allowed: hasCapability(capability),
    error: manifestError,
    retry: () => {
      void reloadManifest();
    },
  };
}
