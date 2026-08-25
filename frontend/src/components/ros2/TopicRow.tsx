import { useNavigate } from 'react-router';
import type { TopicInfo } from '../../types/topic.ts';
import { TypeBadge } from './TypeBadge.tsx';

export function TopicRow({ topic }: { topic: TopicInfo }) {
  const navigate = useNavigate();

  return (
    <button
      onClick={() => navigate(`/ros2/topics/detail?topic=${encodeURIComponent(topic.name)}`)}
      className="w-full flex items-center justify-between gap-4 p-3 rounded-row bg-surface-1 hover:bg-surface-2 border border-border-soft hover:border-line transition-colors duration-fast text-left focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60"
    >
      <span className="font-mono text-sm text-ink break-all">{topic.name}</span>
      <TypeBadge type={topic.type} />
    </button>
  );
}
