import type { ReactNode } from 'react';

import type { RosCapability } from '../../hooks/useRosCapability.ts';
import { ErrorState } from '../ui/ErrorState.tsx';
import { LoadingState } from '../ui/LoadingState.tsx';

interface CapabilityGateProps {
  capability: RosCapability;
  noCapability: string;
  children: ReactNode;
}

/** Manifest capability gate: error → loading → not-allowed → content. */
export function CapabilityGate({
  capability,
  noCapability,
  children,
}: CapabilityGateProps) {
  if (capability.error !== null) {
    return <ErrorState message={capability.error} onRetry={capability.retry} />;
  }

  if (!capability.ready) {
    return <LoadingState />;
  }

  if (!capability.allowed) {
    return <p className="text-ink-muted">{noCapability}</p>;
  }

  return children;
}
