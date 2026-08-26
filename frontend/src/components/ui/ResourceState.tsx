import type { ReactNode } from 'react';

import { useApiErrorMessage } from '../../hooks/useApiErrorMessage.ts';
import type { AsyncResource } from '../../hooks/useAsyncResource.ts';
import { EmptyState } from './EmptyState.tsx';
import { ErrorState } from './ErrorState.tsx';
import { LoadingState } from './LoadingState.tsx';

interface ResourceStateProps {
  resource: AsyncResource<readonly unknown[]>;
  emptyMessage: string;
  children: ReactNode;
}

export function ResourceState({ resource, emptyMessage, children }: ResourceStateProps) {
  const errorMessage = useApiErrorMessage();

  if (resource.loading && resource.data === null) {
    return <LoadingState size={24} />;
  }

  if (resource.error !== null) {
    return (
      <ErrorState message={errorMessage(resource.error)} onRetry={resource.reload} />
    );
  }

  if (resource.data !== null && resource.data.length === 0) {
    return <EmptyState message={emptyMessage} />;
  }

  return children;
}
