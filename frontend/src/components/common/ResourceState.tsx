import type { ReactNode } from 'react';

import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import type { AsyncResource } from '@/hooks/useAsyncResource';
import { EmptyState } from './EmptyState.tsx';
import { ErrorState } from './ErrorState.tsx';
import { LoadingState } from './LoadingState.tsx';

interface ResourceStateProps<T> {
  resource: AsyncResource<T>;
  emptyMessage?: string;
  isEmpty?: (data: T) => boolean;
  children: (data: T) => ReactNode;
}

function arrayIsEmpty(data: unknown): boolean {
  return Array.isArray(data) && data.length === 0;
}

export function ResourceState<T>({
  resource,
  emptyMessage,
  isEmpty = arrayIsEmpty,
  children,
}: ResourceStateProps<T>) {
  const errorMessage = useApiErrorMessage();

  if (resource.loading && resource.data === null) {
    return <LoadingState className="size-6" />;
  }

  if (resource.error !== null) {
    return (
      <ErrorState message={errorMessage(resource.error)} onRetry={resource.reload} />
    );
  }

  if (resource.data !== null && isEmpty(resource.data)) {
    return emptyMessage !== undefined ? <EmptyState message={emptyMessage} /> : null;
  }

  return <>{resource.data === null ? null : children(resource.data)}</>;
}
