import type { ServiceEndpoint } from '../../types/service.ts';
import type { TopicEndpoint } from '../../types/topic.ts';
import { TypeBadge } from './TypeBadge.tsx';

function formatPolicy(value: string): string {
  return value.replaceAll('_', ' ');
}

const qosChipClasses =
  'text-xs text-ink-faint bg-surface-3 border border-line rounded-row px-1.5 py-0.5 whitespace-nowrap';

export function TopicEndpointRow({ endpoint }: { endpoint: TopicEndpoint }) {
  return (
    <div className="flex items-center justify-between gap-4 p-3 rounded-row bg-surface-1 border border-border-soft">
      <span className="font-mono text-sm text-ink break-all">{endpoint.info.name}</span>
      <div className="flex items-center gap-2 flex-wrap justify-end shrink-0">
        <TypeBadge type={endpoint.info.type} />
        <span className={qosChipClasses}>depth {endpoint.qos_profile.depth}</span>
        <span className={qosChipClasses}>{formatPolicy(endpoint.qos_profile.reliability)}</span>
        <span className={qosChipClasses}>{formatPolicy(endpoint.qos_profile.durability)}</span>
      </div>
    </div>
  );
}

export function ServiceEndpointRow({ endpoint }: { endpoint: ServiceEndpoint }) {
  return (
    <div className="flex items-center justify-between gap-4 p-3 rounded-row bg-surface-1 border border-border-soft">
      <span className="font-mono text-sm text-ink break-all">{endpoint.info.name}</span>
      <TypeBadge type={endpoint.info.type} />
    </div>
  );
}
