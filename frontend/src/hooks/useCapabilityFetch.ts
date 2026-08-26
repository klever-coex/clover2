import { useEffect, useEffectEvent } from 'react';

import type { RosCapability } from './useRosCapability.ts';

export function useCapabilityFetch(
  capability: RosCapability,
  reload: () => void | Promise<void>,
): void {
  const runReload = useEffectEvent(reload);

  useEffect(() => {
    if (capability.ready && capability.allowed) {
      void runReload();
    }
  }, [capability.ready, capability.allowed]);
}
