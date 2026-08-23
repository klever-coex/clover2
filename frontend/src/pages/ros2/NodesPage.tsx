import { useTranslation } from 'react-i18next';
import { ListPage } from '../../components/ros2/ListPage.tsx';
import { NodeRow } from '../../components/ros2/NodeRow.tsx';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useRosStore } from '../../store/useRosStore.ts';

/** Node list with search; gated on the backend 'nodes' capability. */
export function NodesPage() {
  const { t } = useTranslation();
  const capability = useRosCapability('nodes');
  const nodes = useRosStore((s) => s.nodes);
  const nodesLoading = useRosStore((s) => s.nodesLoading);
  const nodesError = useRosStore((s) => s.nodesError);
  const reloadNodes = useRosStore((s) => s.reloadNodes);

  return (
    <ListPage<string>
      title={t('nodes.title')}
      capability={capability}
      noCapability={t('nodes.noCapability')}
      searchPlaceholder={t('nodes.searchPlaceholder')}
      emptyMessage={t('nodes.empty')}
      items={nodes}
      loading={nodesLoading}
      error={nodesError?.message ?? null}
      onReload={reloadNodes}
      filter={(name, q) => name.toLowerCase().includes(q)}
      keyOf={(name) => name}
      renderItem={(name) => <NodeRow name={name} />}
    />
  );
}
