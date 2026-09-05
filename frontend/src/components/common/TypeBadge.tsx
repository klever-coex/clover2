import { Badge } from '@/components/ui/badge';

export function TypeBadge({ type }: { type: string }) {
  return (
    <Badge variant="secondary" className="font-mono">
      {type}
    </Badge>
  );
}
