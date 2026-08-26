import { Video } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { useNavigate } from 'react-router';

import type { TopicInfo } from '../../types/topic.ts';
import { isVideoTopic } from '../../utils/videoStream.ts';
import { IconButton } from '../ui/IconButton.tsx';
import { TypeBadge } from './TypeBadge.tsx';

export function TopicRow({ topic }: { topic: TopicInfo }) {
  const navigate = useNavigate();
  const { t } = useTranslation();
  const isVideo = isVideoTopic(topic.type);

  return (
    <div className="flex items-stretch gap-1 rounded-row bg-surface-1 hover:bg-surface-2 border border-border-soft hover:border-line transition-colors duration-fast">
      <button
        onClick={() => navigate(`/ros2/topics/detail?topic=${encodeURIComponent(topic.name)}`)}
        className="flex-1 flex items-center justify-between gap-4 p-3 rounded-row text-left focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60"
      >
        <span className="font-mono text-sm text-ink break-all">{topic.name}</span>
        <TypeBadge type={topic.type} />
      </button>
      {isVideo && (
        <div className="flex items-center pr-1">
          <IconButton
            icon={Video}
            label={t('video.openVideo')}
            variant="ghost"
            onClick={() => navigate(`/video?topic=${encodeURIComponent(topic.name)}`)}
          />
        </div>
      )}
    </div>
  );
}
