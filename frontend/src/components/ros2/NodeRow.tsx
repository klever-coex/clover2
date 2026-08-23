import { useNavigate } from 'react-router';
import { ChevronRight } from 'lucide-react';

export function NodeRow({ name }: { name: string }) {
  const navigate = useNavigate();

  return (
    <button
      onClick={() => navigate(`/ros2/nodes/detail?node=${encodeURIComponent(name)}`)}
      className="w-full flex items-center justify-between gap-4 p-3 rounded-row bg-surface-1 hover:bg-surface-2 border border-border-soft hover:border-line transition-colors duration-fast text-left focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-royal/60"
    >
      <span className="font-mono text-sm text-ink break-all">{name}</span>
      <ChevronRight size={16} className="text-ink-faint shrink-0" />
    </button>
  );
}
