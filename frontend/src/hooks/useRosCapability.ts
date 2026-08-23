import { useEffect } from 'react';

import { useRosStore } from '../store/useRosStore.ts';
import type { Capability } from '../types/manifest.ts';

export interface RosCapability {
  ready: boolean;
  allowed: boolean;
  error: string | null;
  retry: () => void;
}

export function useRosCapability(capability: Capability): RosCapability {
  const manifest = useRosStore((s) => s.manifest);
  const manifestLoading = useRosStore((s) => s.manifestLoading);
  const manifestError = useRosStore((s) => s.manifestError);
  const fetchManifest = useRosStore((s) => s.fetchManifest);
  const hasCapability = useRosStore((s) => s.hasCapability);

  useEffect(() => {
    if (manifest === null && !manifestLoading) {
      void fetchManifest();
    }
  }, [manifest, manifestLoading, fetchManifest]);

  return {
    ready: manifest !== null,
    allowed: hasCapability(capability),
    error: manifestError?.message ?? null,
    retry: () => {
      void fetchManifest();
    },
  };
}
