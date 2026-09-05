import type { ReactNode } from 'react';

import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import type { RosCapability } from '@/hooks/useRosCapability';
import { ErrorState } from '../common/ErrorState.tsx';
import { LoadingState } from '../common/LoadingState.tsx';

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
    return <p className="text-muted-foreground">{noCapability}</p>;
  }

  return children;
}
