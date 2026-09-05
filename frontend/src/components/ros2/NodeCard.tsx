import { useNavigate } from 'react-router';

import type { NodeInfo } from '@/types/node';
import { Badge } from '@/components/ui/badge';
import { LifecycleBadge } from './LifecycleBadge.tsx';

interface NodeCardProps {
  name: string;
  info?: NodeInfo;
}

export function NodeCard({ name, info }: NodeCardProps) {
  const navigate = useNavigate();

  return (
    <button
      onClick={() => navigate(`/ros2/nodes/detail?node=${encodeURIComponent(name)}`)}
      className="w-full flex flex-col items-start gap-2 p-3 rounded-row bg-card hover:bg-muted border border-border transition-colors duration-fast text-left focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring/60"
    >
      <span className="font-mono text-sm text-foreground break-all">{name}</span>
      {info !== undefined && (
        <span className="flex items-center flex-wrap gap-2 mt-auto">
          {info.ns !== '/' && (
            <Badge variant="secondary" className="font-mono">
              {info.ns}
            </Badge>
          )}
          <LifecycleBadge state={info.lifecycle_state} />
        </span>
      )}
    </button>
  );
}
