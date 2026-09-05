import type { ReactNode } from 'react';

import type { AsyncResource } from '@/hooks/useAsyncResource';
import type { ServiceEndpoint } from '@/types/service';
import type { TopicEndpoint } from '@/types/topic';
import { CollapsibleCard } from '../common/CollapsibleCard.tsx';
import { ResourceState } from '../common/ResourceState.tsx';
import { ServiceEndpointRow, TopicEndpointRow } from './EndpointRow.tsx';

interface EndpointPanelProps<T> {
  title: string;
  empty: string;
  resource: AsyncResource<T[]>;
  defaultOpen?: boolean;
  children: (data: T[]) => ReactNode;
}

function EndpointPanel<T>({
  title,
  empty,
  resource,
  defaultOpen,
  children,
}: EndpointPanelProps<T>) {
  const count = resource.data?.length;

  return (
    <CollapsibleCard title={title} count={count} defaultOpen={defaultOpen}>
      <ResourceState resource={resource} emptyMessage={empty}>
        {(data) => <div className="flex flex-col gap-1">{children(data)}</div>}
      </ResourceState>
    </CollapsibleCard>
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
      {(data) =>
        data.map((endpoint) => <TopicEndpointRow key={endpoint.info.name} endpoint={endpoint} />)
      }
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
      {(data) =>
        data.map((endpoint) => <ServiceEndpointRow key={endpoint.info.name} endpoint={endpoint} />)
      }
    </EndpointPanel>
  );
}
