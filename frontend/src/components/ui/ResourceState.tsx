import type { ReactNode } from 'react';

import type { AsyncResource } from '../../hooks/useAsyncResource.ts';
import { EmptyState } from './EmptyState.tsx';
import { ErrorState } from './ErrorState.tsx';
import { LoadingState } from './LoadingState.tsx';

interface ResourceStateProps {
  resource: AsyncResource<readonly unknown[]>;
  emptyMessage: string;
  children: ReactNode;
}

/**
 * Canonical 4-state renderer for an async-loaded list resource:
 * loading → error → empty → children (the loaded list).
 */
export function ResourceState({ resource, emptyMessage, children }: ResourceStateProps) {
  if (resource.loading && resource.data === null) {
    return <LoadingState size={24} />;
  }

  if (resource.error !== null) {
    return <ErrorState message={resource.error.message} onRetry={resource.reload} />;
  }

  if (resource.data !== null && resource.data.length === 0) {
    return <EmptyState message={emptyMessage} />;
  }

  return children;
}
