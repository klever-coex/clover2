import { useState } from 'react';
import { useTranslation } from 'react-i18next';

import { ListPage } from '../../components/ros2/ListPage.tsx';
import { NodeCard } from '../../components/ros2/NodeCard.tsx';
import { SortSelect } from '../../components/ros2/SortSelect.tsx';
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
  const [sort, setSort] = useState<'name' | 'lifecycle'>('name');

  return (
    <ListPage<string>
      title={t('nodes.title')}
      capability={capability}
      noCapability={t('nodes.noCapability')}
      searchPlaceholder={t('nodes.searchPlaceholder')}
      emptyMessage={t('nodes.empty')}
      items={nodes}
      loading={nodesLoading}
      error={nodesError}
      onReload={reloadNodes}
      filter={(name, q) => name.toLowerCase().includes(q)}
      keyOf={(name) => name}
      renderItem={(name) => <NodeCard name={name} info={infos.data?.get(name)} />}
      itemsClassName="mt-4 grid gap-3 grid-cols-[repeat(auto-fill,minmax(240px,1fr))]"
      toolbarExtra={
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
      sortItems={(items) => {
        const sorted = [...items];
        sorted.sort((a, b) => {
          if (sort === 'name') return a.localeCompare(b);
          const lifeA = infos.data?.get(a)?.is_lifecycle === true;
          const lifeB = infos.data?.get(b)?.is_lifecycle === true;
          return Number(lifeB) - Number(lifeA) || a.localeCompare(b);
        });
        return sorted;
      }}
    />
  );
}
