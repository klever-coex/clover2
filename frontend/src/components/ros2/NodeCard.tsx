import { useNavigate } from 'react-router';

import type { NodeInfo } from '@/types/node';
import { Badge } from '@/components/ui/badge';
import { Skeleton } from '@/components/ui/skeleton';
import { CardRow } from '../common/CardRow.tsx';
import { LifecycleBadge } from './LifecycleBadge.tsx';

interface NodeCardProps {
  name: string;
  info?: NodeInfo;
}

export function NodeCard({ name, info }: NodeCardProps) {
  const navigate = useNavigate();

  return (
    <CardRow
      onSelect={() => navigate(`/ros2/nodes/detail?node=${encodeURIComponent(name)}`)}
      className="h-full"
      contentClassName="flex-col items-start gap-2"
    >
      <span
        title={name}
        className="font-mono text-sm text-foreground truncate max-w-full"
      >
        {name}
      </span>
      <span className="flex items-center flex-wrap gap-2 mt-auto">
        {info === undefined ? (
          <>
            <Skeleton className="h-5 w-10 rounded-4xl" />
            <Skeleton className="h-5 w-20 rounded-4xl" />
          </>
        ) : (
          <>
            <Badge variant="secondary" className="font-mono">
              {info.ns}
            </Badge>
            <LifecycleBadge state={info.lifecycle_state?.label} />
          </>
        )}
      </span>
    </CardRow>
  );
}
