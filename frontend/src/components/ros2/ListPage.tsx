import { useState } from 'react';
import type { ReactNode } from 'react';

import { useApiErrorMessage } from '../../hooks/useApiErrorMessage.ts';
import { useCapabilityFetch } from '../../hooks/useCapabilityFetch.ts';
import type { RosCapability } from '../../hooks/useRosCapability.ts';
import type { ApiError } from '../../types/errors.ts';
import { EmptyState } from '../ui/EmptyState.tsx';
import { ErrorState } from '../ui/ErrorState.tsx';
import { LoadingState } from '../ui/LoadingState.tsx';
import { PageHeader } from '../ui/PageHeader.tsx';
import { CapabilityGate } from './CapabilityGate.tsx';
import { ListToolbar } from './ListToolbar.tsx';

interface ListPageProps<T> {
  title: string;
  capability: RosCapability;
  noCapability: string;
  searchPlaceholder: string;
  emptyMessage: string;
  items: readonly T[];
  loading: boolean;
  error: ApiError | null;
  onReload: () => void | Promise<void>;
  /** Query is already trimmed and lowercased. */
  filter: (item: T, query: string) => boolean;
  keyOf: (item: T) => string;
  renderItem: (item: T) => ReactNode;
  toolbarExtra?: ReactNode;
  sortItems?: (items: readonly T[]) => readonly T[];
  /** Classes of the items container; defaults to a vertical list. */
  itemsClassName?: string;
}

export function ListPage<T>({
  title,
  capability,
  noCapability,
  searchPlaceholder,
  emptyMessage,
  items,
  loading,
  error,
  onReload,
  filter,
  keyOf,
  renderItem,
  toolbarExtra,
  sortItems,
  itemsClassName = 'mt-4 space-y-1',
}: ListPageProps<T>) {
  const [query, setQuery] = useState('');
  const errorMessage = useApiErrorMessage();

  useCapabilityFetch(capability, onReload);

  const q = query.trim().toLowerCase();
  const filtered = q === '' ? items : items.filter((item) => filter(item, q));
  const visible = sortItems === undefined ? filtered : sortItems(filtered);

  return (
    <div className="p-6">
      <PageHeader title={title} />

      <div className="mt-4">
        <CapabilityGate capability={capability} noCapability={noCapability}>
          <ListToolbar
            value={query}
            onChange={setQuery}
            placeholder={searchPlaceholder}
            extra={toolbarExtra}
          />

          {loading && items.length === 0 && <LoadingState />}

          {error !== null && (
            <ErrorState message={errorMessage(error)} onRetry={onReload} />
          )}

          {error === null && !loading && visible.length === 0 && (
            <EmptyState message={emptyMessage} />
          )}

          {visible.length > 0 && (
            <div className={itemsClassName}>
              {visible.map((item) => (
                <div key={keyOf(item)}>{renderItem(item)}</div>
              ))}
            </div>
          )}
        </CapabilityGate>
      </div>
    </div>
  );
}
