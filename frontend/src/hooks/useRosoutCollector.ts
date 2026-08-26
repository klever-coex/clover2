import { useEffect } from 'react';

import { useRosStore } from '../store/useRosStore.ts';
import { useRosCapability } from './useRosCapability.ts';

/** Starts the background /rosout collector once the backend confirms the topics capability. */
export function useRosoutCollector(): void {
  const capability = useRosCapability('topics');
  const startRosoutStream = useRosStore((s) => s.startRosoutStream);

  useEffect(() => {
    if (capability.ready && capability.allowed) {
      startRosoutStream();
    }
  }, [capability.ready, capability.allowed, startRosoutStream]);
}
