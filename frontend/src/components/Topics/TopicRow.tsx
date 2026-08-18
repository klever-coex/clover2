import { useNavigate } from 'react-router';
import type { TopicInfo } from '../../types/ros.ts';

export function TopicRow({ topic }: { topic: TopicInfo }) {
  const navigate = useNavigate();

  return (
    <button
      onClick={() => navigate(`/topics/detail?topic=${encodeURIComponent(topic.name)}`)}
      className="w-full flex items-center justify-between gap-4 p-3 rounded-xl hover:bg-gray-200 transition text-left"
    >
      <span className="font-mono text-sm">{topic.name}</span>
      <span className="text-xs text-gray-500 bg-gray-200 px-2 py-0.5 rounded">{topic.type}</span>
    </button>
  );
}
