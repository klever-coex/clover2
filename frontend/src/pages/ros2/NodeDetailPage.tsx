import { useTranslation } from 'react-i18next';
import { Navigate, useSearchParams } from 'react-router';
import { ServicePanel, TopicPanel } from '../../components/ros2/EndpointPanels.tsx';
import { Badge } from '../../components/ui/Badge.tsx';
import { EmptyState } from '../../components/ui/EmptyState.tsx';
import { ErrorState } from '../../components/ui/ErrorState.tsx';
import { LoadingState } from '../../components/ui/LoadingState.tsx';
import { PageHeader } from '../../components/ui/PageHeader.tsx';
import { Panel } from '../../components/ui/Panel.tsx';
import { useNodeResources } from '../../hooks/useNodeResources.ts';
import type { AsyncResource } from '../../hooks/useAsyncResource.ts';
import type { NodeInfo } from '../../types/node.ts';

/** Node detail; the node name comes from the ?node= query parameter. */
export function NodeDetailPage() {
  const [searchParams] = useSearchParams();
  const nodeName = searchParams.get('node');

  if (nodeName === null) {
    return <Navigate to="/ros2/nodes" replace />;
  }

  // Key remounts the view when switching nodes, resetting all resources.
  return <NodeDetailView key={nodeName} nodeName={nodeName} />;
}

function NodeDetailView({ nodeName }: { nodeName: string }) {
  const { t } = useTranslation();
  const resources = useNodeResources(nodeName);

  return (
    <div className="p-6">
      <PageHeader
        title={<span className="font-mono">{nodeName}</span>}
        backTo={{ to: '/ros2/nodes', label: t('nodes.backToNodes') }}
      />

      <div className="flex items-center gap-3 mt-4 flex-wrap">
        {resources.info.data !== null && (
          <Badge tone={resources.info.data.is_lifecycle ? 'accent' : 'neutral'}>
            {resources.info.data.is_lifecycle ? t('nodes.lifecycle') : t('nodes.regular')}
          </Badge>
        )}
        {resources.info.data !== null && resources.info.data.ns !== '/' && (
          <span className="text-sm text-ink-muted">
            {t('nodes.namespace')}:{' '}
            <span className="font-mono">{resources.info.data.ns}</span>
          </span>
        )}
      </div>

      <div className="grid grid-cols-1 gap-4 mt-6">
        <InfoPanel resource={resources.info} />
        <TopicPanel
          title={t('nodes.publishers')}
          empty={t('nodes.noPublishers')}
          resource={resources.publishers}
        />
        <TopicPanel
          title={t('nodes.subscribers')}
          empty={t('nodes.noSubscribers')}
          resource={resources.subscribes}
        />
        <ServicePanel
          title={t('nodes.servers')}
          empty={t('nodes.noServers')}
          resource={resources.servers}
        />
        <ServicePanel
          title={t('nodes.clients')}
          empty={t('nodes.noClients')}
          resource={resources.clients}
        />
      </div>
    </div>
  );
}

function InfoPanel({ resource }: { resource: AsyncResource<NodeInfo> }) {
  const { t } = useTranslation();

  return (
    <Panel title={t('nodes.info')}>
      {resource.loading && resource.data === null && <LoadingState size={24} />}
      {resource.error !== null && (
        <ErrorState message={resource.error.message} onRetry={resource.reload} />
      )}
      {resource.data !== null && (
        <div className="space-y-2">
          <InfoRow label={t('nodes.name')} value={resource.data.name} />
          <InfoRow label={t('nodes.namespace')} value={resource.data.ns} />
          <InfoRow
            label={t('nodes.lifecycle')}
            value={
              resource.data.is_lifecycle ? t('nodes.lifecycle') : t('nodes.regular')
            }
          />
        </div>
      )}
      {resource.data === null && resource.error === null && !resource.loading && (
        <EmptyState message={t('nodes.noCapability')} />
      )}
    </Panel>
  );
}

function InfoRow({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex items-center justify-between gap-4">
      <span className="text-sm text-ink-muted">{label}</span>
      <span className="font-mono text-sm text-ink break-all text-right">{value}</span>
    </div>
  );
}
