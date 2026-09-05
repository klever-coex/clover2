import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { Navigate, useSearchParams } from 'react-router';
import { ServicePanel, TopicPanel } from '../../components/ros2/EndpointPanels.tsx';
import { LifecycleBadge } from '../../components/ros2/LifecycleBadge.tsx';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { ResourceState } from '../../components/common/ResourceState.tsx';
import { useNodeResources } from '@/hooks/useNodeResources';
import type { AsyncResource } from '@/hooks/useAsyncResource';
import type { NodeInfo } from '@/types/node';
import { usePageHeader } from '@/store/usePageHeader';
import { route } from '@/routes/navigation.ts';

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

  usePageHeader([
    { label: t(route('/ros2/nodes').labelKey), to: '/ros2/nodes' },
    { label: nodeName, mono: true },
  ]);

  return (
    <div className="p-6">
      <div className="grid grid-cols-1 gap-4">
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

  return (
    <Card>
      <CardHeader>
        <CardTitle>{t('nodes.info')}</CardTitle>
      </CardHeader>
      <CardContent>
        <ResourceState resource={resource}>
          {(data) => (
            <div className="flex flex-col gap-2">
              <InfoRow label={t('nodes.name')} value={data.name} />
              <InfoRow label={t('nodes.namespace')} value={data.ns} />
              <InfoRow
                label={t('nodes.lifecycle')}
                value={<LifecycleBadge state={data.lifecycle_state} />}
              />
            </div>
          )}
        </ResourceState>
      </CardContent>
    </Card>
  );
}

function InfoRow({ label, value }: { label: string; value: ReactNode }) {
  return (
    <div className="flex items-center justify-between gap-4">
      <span className="text-sm text-muted-foreground">{label}</span>
      <span className="text-sm text-foreground break-all text-right">{value}</span>
    </div>
  );
}
