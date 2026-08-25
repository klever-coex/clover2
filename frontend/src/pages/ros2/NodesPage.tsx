import { useEffect, useEffectEvent, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { ListToolbar } from '../../components/ros2/ListToolbar.tsx';
import { NodeCard } from '../../components/ros2/NodeCard.tsx';
import { SortSelect } from '../../components/ros2/SortSelect.tsx';
import { EmptyState } from '../../components/ui/EmptyState.tsx';
import { ErrorState } from '../../components/ui/ErrorState.tsx';
import { LoadingState } from '../../components/ui/LoadingState.tsx';
import { PageHeader } from '../../components/ui/PageHeader.tsx';
import { useNodeInfos } from '../../hooks/useNodeInfos.ts';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useRosStore } from '../../store/useRosStore.ts';

export function NodesPage() {
  const { t } = useTranslation();
  const capability = useRosCapability('nodes');
  const nodes = useRosStore((s) => s.nodes);
  const nodesLoading = useRosStore((s) => s.nodesLoading);
  const nodesError = useRosStore((s) => s.nodesError);
  const reloadNodes = useRosStore((s) => s.reloadNodes);

  const infos = useNodeInfos(nodes);
  const [query, setQuery] = useState('');
  const [sort, setSort] = useState<'name' | 'lifecycle'>('name');

  const runReload = useEffectEvent(reloadNodes);

  useEffect(() => {
    if (capability.ready && capability.allowed) {
      void runReload();
    }
  }, [capability.ready, capability.allowed]);

  const q = query.trim().toLowerCase();
  const filtered =
    q === '' ? nodes : nodes.filter((name) => name.toLowerCase().includes(q));

  const sorted = [...filtered].sort((a, b) => {
    if (sort === 'name') return a.localeCompare(b);
    const lifeA = infos.data?.get(a)?.is_lifecycle === true;
    const lifeB = infos.data?.get(b)?.is_lifecycle === true;
    return Number(lifeB) - Number(lifeA) || a.localeCompare(b);
  });

  return (
    <div className="p-6">
      <PageHeader title={t('nodes.title')} />

      <div className="mt-4">
        <CapabilityGate capability={capability} noCapability={t('nodes.noCapability')}>
          <ListToolbar
            value={query}
            onChange={setQuery}
            placeholder={t('nodes.searchPlaceholder')}
            extra={
              <SortSelect
                ariaLabel={t('common.sortBy')}
                value={sort}
                onChange={(value) => setSort(value as 'name' | 'lifecycle')}
                options={[
                  { value: 'name', label: t('nodes.sortName') },
                  { value: 'lifecycle', label: t('nodes.sortLifecycle') },
                ]}
              />
            }
          />

          {nodesLoading && nodes.length === 0 && <LoadingState />}

          {nodesError !== null && (
            <ErrorState message={nodesError.message} onRetry={reloadNodes} />
          )}

          {nodesError === null && !nodesLoading && filtered.length === 0 && (
            <EmptyState message={t('nodes.empty')} />
          )}

          {sorted.length > 0 && (
            <div className="mt-4 grid gap-3 grid-cols-[repeat(auto-fill,minmax(240px,1fr))]">
              {sorted.map((name) => (
                <NodeCard key={name} name={name} info={infos.data?.get(name)} />
              ))}
            </div>
          )}
        </CapabilityGate>
      </div>
    </div>
  );
}
