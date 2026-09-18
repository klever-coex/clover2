import { Video } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { useNavigate } from 'react-router';

import type { TopicInfo } from '@/types/topic';
import { isVideoTopic } from '../../utils/videoStream.ts';
import { CardRow } from '../common/CardRow.tsx';
import { TypeBadge } from '../common/TypeBadge.tsx';
import { TooltipButton } from '../common/TooltipButton.tsx';

export function TopicRow({ topic }: { topic: TopicInfo }) {
  const navigate = useNavigate();
  const { t } = useTranslation();
  const isVideo = isVideoTopic(topic.type);

  return (
    <CardRow
      onSelect={() => navigate(`/ros2/topics/detail?topic=${encodeURIComponent(topic.name)}`)}
      action={
        isVideo ? (
          <TooltipButton
            variant="ghost"
            size="icon"
            label={t('video.openVideo')}
            onClick={() => navigate(`/video?topic=${encodeURIComponent(topic.name)}`)}
          >
            <Video />
          </TooltipButton>
        ) : undefined
      }
    >
      <span className="font-mono text-sm text-foreground break-all">{topic.name}</span>
      <TypeBadge type={topic.type} />
    </CardRow>
  );
}
