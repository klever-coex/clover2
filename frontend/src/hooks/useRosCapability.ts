import { useEffect } from 'react';

import { useRosStore } from '../store/useRosStore.ts';
import type { ApiError } from '../types/errors.ts';
import type { Capability } from '../types/manifest.ts';

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
  const fetchManifest = useRosStore((s) => s.fetchManifest);
  const hasCapability = useRosStore((s) => s.hasCapability);

  useEffect(() => {
    if (manifest === null && !manifestLoading && manifestError === null) {
      void fetchManifest();
    }
  }, [manifest, manifestLoading, manifestError, fetchManifest]);

  return {
    ready: manifest !== null,
    allowed: hasCapability(capability),
    error: manifestError,
    retry: () => {
      void fetchManifest();
    },
  };
}
