import { useEffect } from 'react';

import { useRosStore } from '@/store/useRosStore';
import { useRosCapability } from './useRosCapability.ts';

export function useRosoutCollector(): void {
  const capability = useRosCapability('topics');
  const startRosoutStream = useRosStore((s) => s.startRosoutStream);

  useEffect(() => {
    if (capability.ready && capability.allowed) {
      startRosoutStream();
    }
  }, [capability.ready, capability.allowed, startRosoutStream]);
}
