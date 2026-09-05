import type { ReactNode } from 'react';
import { ChevronDown } from 'lucide-react';

import type { AsyncResource } from '@/hooks/useAsyncResource';
import type { ServiceEndpoint } from '@/types/service';
import type { TopicEndpoint } from '@/types/topic';
import { Badge } from '@/components/ui/badge';
import {
  Card,
  CardContent,
  CardHeader,
  CardTitle,
} from '@/components/ui/card';
import {
  Collapsible,
  CollapsibleContent,
  CollapsibleTrigger,
} from '@/components/ui/collapsible';
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
    <Collapsible defaultOpen={defaultOpen}>
      <Card>
        <CardHeader>
          <CollapsibleTrigger className="group/collapsible-trigger flex w-full items-center gap-2 text-left">
            <CardTitle>{title}</CardTitle>
            {count !== undefined && <Badge variant="secondary">{count}</Badge>}
            <ChevronDown className="ml-auto size-4 shrink-0 text-muted-foreground transition-transform duration-fast group-data-[state=open]/collapsible-trigger:rotate-180" />
          </CollapsibleTrigger>
        </CardHeader>
        <CollapsibleContent>
          <CardContent>
            <ResourceState resource={resource} emptyMessage={empty}>
              {(data) => <div className="flex flex-col gap-1">{children(data)}</div>}
            </ResourceState>
          </CardContent>
        </CollapsibleContent>
      </Card>
    </Collapsible>
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
