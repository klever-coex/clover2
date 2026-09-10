import type { ReactNode } from 'react';

import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useCapabilityFetch } from '@/hooks/useCapabilityFetch';
import { useRosCapability } from '@/hooks/useRosCapability';
import type { Capability } from '@/types/manifest';
import { ErrorState } from '../common/ErrorState.tsx';
import { LoadingState } from '../common/LoadingState.tsx';

interface CapabilityGateProps {
  /** Backend capability required to render children. */
  name: Capability;
  /** Called once the capability is confirmed (e.g. initial data load). */
  onReady?: () => void | Promise<void>;
  noCapability: string;
  children: ReactNode;
}

export function CapabilityGate({ name, onReady, noCapability, children }: CapabilityGateProps) {
  const errorMessage = useApiErrorMessage();
  const capability = useRosCapability(name);
  useCapabilityFetch(capability, () => onReady?.());

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
