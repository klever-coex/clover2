import type { ReactNode } from 'react';

import type { AsyncResource } from '../../hooks/useAsyncResource.ts';
import type { ServiceEndpoint } from '../../types/service.ts';
import type { TopicEndpoint } from '../../types/topic.ts';
import { Panel } from '../ui/Panel.tsx';
import { ResourceState } from '../ui/ResourceState.tsx';
import { ServiceEndpointRow, TopicEndpointRow } from './EndpointRow.tsx';

interface EndpointPanelProps {
  title: string;
  empty: string;
  resource: AsyncResource<readonly unknown[]>;
  defaultOpen?: boolean;
  children: ReactNode;
}

function EndpointPanel({ title, empty, resource, defaultOpen, children }: EndpointPanelProps) {
  return (
    <Panel title={title} count={resource.data?.length} collapsible defaultOpen={defaultOpen}>
      <ResourceState resource={resource} emptyMessage={empty}>
        <div className="space-y-1">{children}</div>
      </ResourceState>
    </Panel>
  );
}

export function TopicPanel({
  title,
  empty,
  resource,
  defaultOpen,
}: {
  title: string;
  empty: string;
  resource: AsyncResource<TopicEndpoint[]>;
  defaultOpen?: boolean;
}) {
  return (
    <EndpointPanel title={title} empty={empty} resource={resource} defaultOpen={defaultOpen}>
      {resource.data?.map((endpoint) => (
        <TopicEndpointRow key={endpoint.info.name} endpoint={endpoint} />
      ))}
    </EndpointPanel>
  );
}

export function ServicePanel({
  title,
  empty,
  resource,
  defaultOpen,
}: {
  title: string;
  empty: string;
  resource: AsyncResource<ServiceEndpoint[]>;
  defaultOpen?: boolean;
}) {
  return (
    <EndpointPanel title={title} empty={empty} resource={resource} defaultOpen={defaultOpen}>
      {resource.data?.map((endpoint) => (
        <ServiceEndpointRow key={endpoint.info.name} endpoint={endpoint} />
      ))}
    </EndpointPanel>
  );
}
