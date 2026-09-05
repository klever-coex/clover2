import { useEffect, useMemo } from 'react';
import { Video } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { Navigate, useNavigate, useSearchParams } from 'react-router';
import { MessageViewer } from '../../components/ros2/MessageViewer.tsx';
import { StatusBadge } from '../../components/ros2/StatusBadge.tsx';
import { TypeBadge } from '../../components/common/TypeBadge.tsx';
import { Button } from '@/components/ui/button';
import { EmptyState } from '../../components/common/EmptyState.tsx';
import { ErrorState } from '../../components/common/ErrorState.tsx';
import { LoadingState } from '../../components/common/LoadingState.tsx';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useTopicStream } from '@/hooks/useTopicStream';
import { usePageHeader } from '@/store/usePageHeader';
import { route } from '@/routes/navigation.ts';
import { useRosStore } from '@/store/useRosStore';
import { cn } from '@/lib/utils';
import { centerFill } from '@/lib/uiStyles';
import { isVideoTopic } from '../../utils/videoStream.ts';

export function TopicDetailPage() {
  const { t } = useTranslation();
  const apiErrorMessage = useApiErrorMessage();
  const navigate = useNavigate();
  const [searchParams] = useSearchParams();
  const topicName = searchParams.get('topic');
  const topics = useRosStore((s) => s.topics);
  const reloadTopics = useRosStore((s) => s.reloadTopics);

  const stream = useTopicStream(topicName);

  const topic = useMemo(
    () => topics.find((candidate) => candidate.name === topicName) ?? null,
    [topics, topicName],
  );

  usePageHeader(
    [
      { label: t(route('/ros2/topics').labelKey), to: '/ros2/topics' },
      { label: topicName ?? '', mono: true },
    ],
    <div className="flex gap-2">
      {topic !== null && isVideoTopic(topic.type) && (
        <Button
          variant="secondary"
          size="sm"
          onClick={() => navigate(`/video?topic=${encodeURIComponent(topic.name)}`)}
        >
          <Video />
          {t('video.openVideo')}
        </Button>
      )}
      <Button variant="secondary" size="sm" onClick={stream.clear}>
        {t('topicDetail.clear')}
      </Button>
      <Button size="sm" onClick={stream.retry}>
        {t('common.retry')}
      </Button>
    </div>,
  );

  useEffect(() => {
    if (topics.length === 0) {
      void reloadTopics();
    }
  }, [topics.length, reloadTopics]);

  if (topicName === null) {
    return <Navigate to="/ros2/topics" replace />;
  }

  const errorMessage =
    stream.error === null
      ? null
      : stream.error.status === 1008
        ? t('topicDetail.unknownTopic')
        : apiErrorMessage(stream.error);

  return (
    <div className="p-6">
      <div className="flex items-center gap-3 flex-wrap">
        <StatusBadge state={stream.state} />
        {topic !== null && <TypeBadge type={topic.type} />}
        <span className="text-xs text-muted-foreground">
          {stream.received} {t('topicDetail.received')}
        </span>
      </div>

      {stream.messages.length > 0 ? (
        <MessageViewer messages={stream.messages} />
      ) : (
        <div className={cn('flex flex-col items-center justify-center', centerFill)}>
          {stream.state !== 'error' && stream.state !== 'closed' && (
            <LoadingState />
          )}
          {stream.state === 'error' && stream.error !== null ? (
            <ErrorState message={errorMessage ?? ''} onRetry={stream.retry} />
          ) : stream.state === 'closed' ? (
            <>
              <EmptyState message={t('topicDetail.closed')} />
              <Button onClick={stream.retry}>
                {t('common.retry')}
              </Button>
            </>
          ) : (
            <EmptyState message={t('topicDetail.noMessages')} />
          )}
        </div>
      )}
    </div>
  );
}
