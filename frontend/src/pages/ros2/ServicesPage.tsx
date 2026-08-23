import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { clover2Api } from '../../api/clover2.ts';
import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { ServicePanel } from '../../components/ros2/EndpointPanels.tsx';
import { EmptyState } from '../../components/ui/EmptyState.tsx';
import { ErrorState } from '../../components/ui/ErrorState.tsx';
import { PageHeader } from '../../components/ui/PageHeader.tsx';
import { SearchInput } from '../../components/ui/SearchInput.tsx';
import { Select } from '../../components/ui/Select.tsx';
import { useAsyncResource } from '../../hooks/useAsyncResource.ts';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useRosStore } from '../../store/useRosStore.ts';

/**
 * Services of a node. The backend has no global service list, so the page is
 * node-driven: pick a node (selector when the 'nodes' capability exists, a
 * text field otherwise) and view its servers and clients.
 */
export function ServicesPage() {
  const { t } = useTranslation();
  const capability = useRosCapability('services');
  const hasNodes = useRosStore((s) => s.hasCapability('nodes'));
  const nodes = useRosStore((s) => s.nodes);
  const nodesError = useRosStore((s) => s.nodesError);
  const reloadNodes = useRosStore((s) => s.reloadNodes);
  const [selectedNode, setSelectedNode] = useState('');

  const effectiveNode = selectedNode !== '' ? selectedNode : (nodes[0] ?? '');
  const enabled = effectiveNode !== '' && capability.allowed;

  const servers = useAsyncResource(
    () => clover2Api.services.servers(effectiveNode),
    [effectiveNode],
    enabled,
  );
  const clients = useAsyncResource(
    () => clover2Api.services.clients(effectiveNode),
    [effectiveNode],
    enabled,
  );

  useEffect(() => {
    if (capability.ready && hasNodes) {
      void reloadNodes();
    }
  }, [capability.ready, hasNodes, reloadNodes]);

  return (
    <div className="p-6">
      <PageHeader title={t('services.title')} />

      <CapabilityGate capability={capability} noCapability={t('services.noCapability')}>
        <div className="flex items-center gap-3 mb-6 flex-wrap">
          <label htmlFor="node-select" className="text-sm text-ink-muted">
            {t('services.selectNode')}
          </label>
          {hasNodes ? (
            <Select
              id="node-select"
              value={effectiveNode}
              onChange={(e) => setSelectedNode(e.target.value)}
              className="flex-1 max-w-md"
            >
              {nodes.map((name) => (
                <option key={name} value={name}>
                  {name}
                </option>
              ))}
            </Select>
          ) : (
            <SearchInput
              className="max-w-md"
              aria-label={t('services.selectNode')}
              value={selectedNode}
              onChange={(e) => setSelectedNode(e.target.value)}
              placeholder={t('services.nodePlaceholder')}
            />
          )}
        </div>

        {hasNodes && nodesError !== null && (
          <ErrorState message={nodesError.message} onRetry={() => void reloadNodes()} />
        )}

        {effectiveNode === '' && <EmptyState message={t('services.empty')} />}

        {effectiveNode !== '' && (
          <div className="grid grid-cols-1 gap-4">
            <ServicePanel
              title={t('services.servers')}
              empty={t('services.noServers')}
              resource={servers}
            />
            <ServicePanel
              title={t('services.clients')}
              empty={t('services.noClients')}
              resource={clients}
            />
          </div>
        )}
      </CapabilityGate>
    </div>
  );
}
