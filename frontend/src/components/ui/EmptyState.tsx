interface EmptyStateProps {
  message: string;
}

export function EmptyState({ message }: EmptyStateProps) {
  return <p className="p-8 text-center text-ink-faint text-sm">{message}</p>;
}
