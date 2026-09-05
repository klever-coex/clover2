import { Video } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { useNavigate } from 'react-router';

import type { TopicInfo } from '@/types/topic';
import { isVideoTopic } from '../../utils/videoStream.ts';
import { TypeBadge } from '../common/TypeBadge.tsx';
import { TooltipButton } from '../common/TooltipButton.tsx';

export function TopicRow({ topic }: { topic: TopicInfo }) {
  const navigate = useNavigate();
  const { t } = useTranslation();
  const isVideo = isVideoTopic(topic.type);

  return (
    <div className="flex items-stretch gap-1 rounded-row bg-card hover:bg-muted border border-border transition-colors duration-fast">
      <button
        onClick={() => navigate(`/ros2/topics/detail?topic=${encodeURIComponent(topic.name)}`)}
        className="flex-1 flex items-center justify-between gap-4 p-3 rounded-row text-left focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring/60"
      >
        <span className="font-mono text-sm text-foreground break-all">{topic.name}</span>
        <TypeBadge type={topic.type} />
      </button>
      {isVideo && (
        <div className="flex items-center pr-1">
          <TooltipButton
            variant="ghost"
            size="icon"
            label={t('video.openVideo')}
            onClick={() => navigate(`/video?topic=${encodeURIComponent(topic.name)}`)}
          >
            <Video />
          </TooltipButton>
        </div>
      )}
    </div>
  );
}
