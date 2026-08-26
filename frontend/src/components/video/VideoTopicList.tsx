import { useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useSearchParams } from 'react-router';

import { cn } from '../../lib/cn.ts';
import { useRosStore } from '../../store/useRosStore.ts';
import { isVideoTopic } from '../../utils/videoStream.ts';
import { ListToolbar } from '../ros2/ListToolbar.tsx';
import { TypeBadge } from '../ros2/TypeBadge.tsx';
import { EmptyState } from '../ui/EmptyState.tsx';

export function VideoTopicList() {
  const { t } = useTranslation();
  const topics = useRosStore((s) => s.topics);
  const [searchParams, setSearchParams] = useSearchParams();
  const [search, setSearch] = useState('');
  const activeTopic = searchParams.get('topic');

  const videoTopics = useMemo(() => {
    const list = topics
      .filter((topic) => isVideoTopic(topic.type))
      .sort((a, b) => a.name.localeCompare(b.name));
    const q = search.trim().toLowerCase();
    return q === '' ? list : list.filter((topic) => topic.name.toLowerCase().includes(q));
  }, [topics, search]);

  return (
    <div className="flex flex-col gap-2 min-h-0">
      <ListToolbar
        value={search}
        onChange={setSearch}
        placeholder={t('video.searchPlaceholder')}
      />
      <div className="min-h-0 overflow-y-auto space-y-0.5">
        {videoTopics.map((topic) => {
          const active = topic.name === activeTopic;
          return (
            <button
              key={topic.name}
              type="button"
              onClick={() => setSearchParams({ topic: topic.name })}
              className={cn(
                'w-full flex items-center justify-between gap-2 px-2 py-1 rounded-row text-left text-xs transition-colors duration-fast cursor-pointer',
                active
                  ? 'bg-surface-3 text-ink'
                  : 'text-ink-muted hover:bg-surface-2 hover:text-ink',
              )}
            >
              <span className="font-mono break-all">{topic.name}</span>
              <TypeBadge type={topic.type} />
            </button>
          );
        })}
        {videoTopics.length === 0 && <EmptyState message={t('video.noTopics')} />}
      </div>
    </div>
  );
}
