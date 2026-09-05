import { Empty, EmptyContent, EmptyDescription } from '@/components/ui/empty.tsx';

interface EmptyStateProps {
  message: string;
}

export function EmptyState({ message }: EmptyStateProps) {
  return (
    <EmptyContent>
      <Empty className="border-none p-8">
        <EmptyDescription className="text-muted-foreground">{message}</EmptyDescription>
      </Empty>
    </EmptyContent>
  );
}
