import { useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useSearchParams } from 'react-router';

import { useRosStore } from '@/store/useRosStore';
import { isVideoTopic } from '../../utils/videoStream.ts';
import { CardRow } from '../common/CardRow.tsx';
import { ListToolbar } from '../common/ListToolbar.tsx';
import { TypeBadge } from '../common/TypeBadge.tsx';
import { EmptyState } from '../common/EmptyState.tsx';

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
      <div className="min-h-0 overflow-y-auto flex flex-col gap-0.5">
        {videoTopics.map((topic) => (
          <CardRow
            key={topic.name}
            variant="compact"
            active={topic.name === activeTopic}
            onSelect={() => setSearchParams({ topic: topic.name })}
            contentClassName="break-all"
          >
            <span className="font-mono">{topic.name}</span>
            <TypeBadge type={topic.type} />
          </CardRow>
        ))}
        {videoTopics.length === 0 && <EmptyState message={t('video.noTopics')} />}
      </div>
    </div>
  );
}
