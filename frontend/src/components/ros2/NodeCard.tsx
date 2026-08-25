import { useNavigate } from 'react-router';

import type { NodeInfo } from '../../types/node.ts';
import { Badge } from '../ui/Badge.tsx';
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
      className="w-full flex flex-col items-start gap-2 p-4 min-h-28 rounded-panel bg-surface-1 hover:bg-surface-2 border border-border-soft hover:border-line transition-colors duration-fast text-left focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60"
    >
      <span className="font-mono text-sm text-ink break-all">{name}</span>
      {info !== undefined && (
        <span className="flex items-center flex-wrap gap-2 mt-auto">
          {info.ns !== '/' && (
            <Badge tone="neutral" className="font-mono">
              {info.ns}
            </Badge>
          )}
          <LifecycleBadge isLifecycle={info.is_lifecycle} state={info.lifecycle_state} />
        </span>
      )}
    </button>
  );
}
