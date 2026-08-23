import { Badge } from '../ui/Badge.tsx';

/** ROS message/service type chip — types are identifiers like names, hence mono. */
export function TypeBadge({ type }: { type: string }) {
  return (
    <Badge tone="neutral" className="font-mono">
      {type}
    </Badge>
  );
}
