import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { Navigate, useSearchParams } from 'react-router';
import { ServicePanel, TopicPanel } from '../../components/ros2/EndpointPanels.tsx';
import { LifecycleBadge } from '../../components/ros2/LifecycleBadge.tsx';
import { EmptyState } from '../../components/ui/EmptyState.tsx';
import { ErrorState } from '../../components/ui/ErrorState.tsx';
import { LoadingState } from '../../components/ui/LoadingState.tsx';
import { PageHeader } from '../../components/ui/PageHeader.tsx';
import { Panel } from '../../components/ui/Panel.tsx';
import { useApiErrorMessage } from '../../hooks/useApiErrorMessage.ts';
import { useNodeResources } from '../../hooks/useNodeResources.ts';
import type { AsyncResource } from '../../hooks/useAsyncResource.ts';
import type { NodeInfo } from '../../types/node.ts';

export function NodeDetailPage() {
  const [searchParams] = useSearchParams();
  const nodeName = searchParams.get('node');

  if (nodeName === null) {
    return <Navigate to="/ros2/nodes" replace />;
  }

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

      <div className="grid grid-cols-1 gap-4 mt-6">
        <InfoPanel resource={resources.info} />
        <TopicPanel
          title={t('nodes.publishers')}
          empty={t('nodes.noPublishers')}
          resource={resources.publishers}
          defaultOpen={false}
        />
        <TopicPanel
          title={t('nodes.subscribers')}
          empty={t('nodes.noSubscribers')}
          resource={resources.subscribes}
          defaultOpen={false}
        />
        <ServicePanel
          title={t('nodes.servers')}
          empty={t('nodes.noServers')}
          resource={resources.servers}
          defaultOpen={false}
        />
        <ServicePanel
          title={t('nodes.clients')}
          empty={t('nodes.noClients')}
          resource={resources.clients}
          defaultOpen={false}
        />
      </div>
    </div>
  );
}

function InfoPanel({ resource }: { resource: AsyncResource<NodeInfo> }) {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();

  return (
    <Panel title={t('nodes.info')}>
      {resource.loading && resource.data === null && <LoadingState size={24} />}
      {resource.error !== null && (
        <ErrorState message={errorMessage(resource.error)} onRetry={resource.reload} />
      )}
      {resource.data !== null && (
        <div className="space-y-2">
          <InfoRow label={t('nodes.name')} value={resource.data.name} />
          <InfoRow label={t('nodes.namespace')} value={resource.data.ns} />
          <InfoRow
            label={t('nodes.lifecycle')}
            value={
              <LifecycleBadge
                state={resource.data.lifecycle_state}
              />
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

function InfoRow({ label, value }: { label: string; value: ReactNode }) {
  return (
    <div className="flex items-center justify-between gap-4">
      <span className="text-sm text-ink-muted">{label}</span>
      <span className="text-sm text-ink break-all text-right">{value}</span>
    </div>
  );
}
