import { useState } from 'react';
import type { ReactNode } from 'react';

import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useCapabilityFetch } from '@/hooks/useCapabilityFetch';
import type { RosCapability } from '@/hooks/useRosCapability';
import type { ApiError } from '@/types/errors';
import { EmptyState } from '../common/EmptyState.tsx';
import { ErrorState } from '../common/ErrorState.tsx';
import { Skeleton } from '@/components/ui/skeleton';
import { CapabilityGate } from './CapabilityGate.tsx';
import { ListToolbar } from '../common/ListToolbar.tsx';

interface ListPageProps<T> {
  capability: RosCapability;
  noCapability: string;
  searchPlaceholder: string;
  emptyMessage: string;
  items: readonly T[];
  loading: boolean;
  error: ApiError | null;
  onReload: () => void | Promise<void>;
  filter: (item: T, query: string) => boolean;
  keyOf: (item: T) => string;
  renderItem: (item: T) => ReactNode;
  toolbarExtra?: ReactNode;
  sortItems?: (items: readonly T[]) => readonly T[];
  itemsClassName?: string;
}

export function ListPage<T>({
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
  itemsClassName = 'mt-4 flex flex-col gap-2',
}: ListPageProps<T>) {
  const [query, setQuery] = useState('');
  const errorMessage = useApiErrorMessage();

  useCapabilityFetch(capability, onReload);

  const q = query.trim().toLowerCase();
  const filtered = q === '' ? items : items.filter((item) => filter(item, q));
  const visible = sortItems === undefined ? filtered : sortItems(filtered);

  return (
    <div className="p-6">
      <div>
        <CapabilityGate capability={capability} noCapability={noCapability}>
          <ListToolbar
            value={query}
            onChange={setQuery}
            placeholder={searchPlaceholder}
            extra={toolbarExtra}
          />

          {loading && items.length === 0 && (
            <div className="mt-4 flex flex-col gap-2" aria-hidden>
              {Array.from({ length: 6 }, (_, index) => (
                <Skeleton key={index} className="h-11 w-full rounded-row" />
              ))}
            </div>
          )}

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
