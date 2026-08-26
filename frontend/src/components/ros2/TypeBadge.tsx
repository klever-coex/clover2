import { Badge } from '../ui/Badge.tsx';

export function TypeBadge({ type }: { type: string }) {
  return (
    <Badge tone="neutral" className="font-mono">
      {type}
    </Badge>
  );
}
