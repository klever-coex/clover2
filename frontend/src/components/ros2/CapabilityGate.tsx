import type { ReactNode } from 'react';

import { useApiErrorMessage } from '../../hooks/useApiErrorMessage.ts';
import type { RosCapability } from '../../hooks/useRosCapability.ts';
import { ErrorState } from '../ui/ErrorState.tsx';
import { LoadingState } from '../ui/LoadingState.tsx';

interface CapabilityGateProps {
  capability: RosCapability;
  noCapability: string;
  children: ReactNode;
}

export function CapabilityGate({
  capability,
  noCapability,
  children,
}: CapabilityGateProps) {
  const errorMessage = useApiErrorMessage();

  if (capability.error !== null) {
    return (
      <ErrorState message={errorMessage(capability.error)} onRetry={capability.retry} />
    );
  }

  if (!capability.ready) {
    return <LoadingState />;
  }

  if (!capability.allowed) {
    return <p className="text-ink-muted">{noCapability}</p>;
  }

  return children;
}
