import { useEffect, useEffectEvent, useState } from 'react';
import type { ReactNode } from 'react';

import type { RosCapability } from '../../hooks/useRosCapability.ts';
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
  error: string | null;
  onReload: () => void;
  /** Query is already trimmed and lowercased. */
  filter: (item: T, query: string) => boolean;
  keyOf: (item: T) => string;
  renderItem: (item: T) => ReactNode;
  /** Optional control (e.g. SortSelect) rendered next to the search input. */
  toolbarExtra?: ReactNode;
  /** Optional sorter applied after filtering. */
  sortItems?: (items: readonly T[]) => readonly T[];
}

/**
 * Canonical searchable resource-list page: capability gate → search (+ optional
 * sort) → loading/error/empty states → row list.
 */
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
}: ListPageProps<T>) {
  const [query, setQuery] = useState('');

  // Effect event: the reload action must not be an effect dependency, otherwise
  // an inline onReload prop would retrigger the fetch on every render.
  const runReload = useEffectEvent(onReload);

  useEffect(() => {
    if (capability.ready && capability.allowed) {
      void runReload();
    }
  }, [capability.ready, capability.allowed]);

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

          {error !== null && <ErrorState message={error} onRetry={onReload} />}

          {error === null && !loading && visible.length === 0 && (
            <EmptyState message={emptyMessage} />
          )}

          {visible.length > 0 && (
            <div className="mt-4 space-y-1">
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
