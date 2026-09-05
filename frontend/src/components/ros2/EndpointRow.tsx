import type { ServiceEndpoint } from '@/types/service';
import type { TopicEndpoint } from '@/types/topic';
import { Badge } from '@/components/ui/badge';
import { Item, ItemActions, ItemContent, ItemTitle } from '@/components/ui/item';
import { TypeBadge } from '../common/TypeBadge.tsx';

function formatPolicy(value: string): string {
  return value.replaceAll('_', ' ');
}

export function TopicEndpointRow({ endpoint }: { endpoint: TopicEndpoint }) {
  return (
    <Item size="sm">
      <ItemContent>
        <ItemTitle className="min-w-0 break-all font-mono leading-5">
          {endpoint.info.name}
        </ItemTitle>
      </ItemContent>
      <ItemActions className="flex-wrap justify-end">
        <TypeBadge type={endpoint.info.type} />
        <Badge variant="secondary" className="font-mono">
          depth {endpoint.qos_profile.depth}
        </Badge>
        <Badge variant="secondary" className="font-mono">
          {formatPolicy(endpoint.qos_profile.reliability)}
        </Badge>
        <Badge variant="secondary" className="font-mono">
          {formatPolicy(endpoint.qos_profile.durability)}
        </Badge>
      </ItemActions>
    </Item>
  );
}

export function ServiceEndpointRow({ endpoint }: { endpoint: ServiceEndpoint }) {
  return (
    <Item size="sm">
      <ItemContent>
        <ItemTitle className="min-w-0 break-all font-mono leading-5">
          {endpoint.info.name}
        </ItemTitle>
      </ItemContent>
      <ItemActions>
        <TypeBadge type={endpoint.info.type} />
      </ItemActions>
    </Item>
  );
}
