import { useTranslation } from 'react-i18next';
import { useSearchParams } from 'react-router';

import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { MseVideoStream } from '../../components/video/MseVideoStream.tsx';
import { VideoTopicList } from '../../components/video/VideoTopicList.tsx';
import { EmptyState } from '../../components/common/EmptyState.tsx';
import { ErrorState } from '../../components/common/ErrorState.tsx';
import { LoadingState } from '../../components/common/LoadingState.tsx';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useCapabilityFetch } from '@/hooks/useCapabilityFetch';
import { useRosCapability } from '@/hooks/useRosCapability';
import { useRosStore } from '@/store/useRosStore';
import { usePageHeader } from '@/store/usePageHeader';
import { route } from '@/routes/navigation.ts';
import { cn } from '@/lib/utils';
import { pageGrid, panelFill } from '@/lib/uiStyles';

export function VideoPage() {
  const { t } = useTranslation();
  const apiErrorMessage = useApiErrorMessage();
  const capability = useRosCapability('topics');
  const topics = useRosStore((s) => s.topics);
  const topicsLoading = useRosStore((s) => s.topicsLoading);
  const topicsError = useRosStore((s) => s.topicsError);
  const reloadTopics = useRosStore((s) => s.reloadTopics);
  const [searchParams] = useSearchParams();
  const topicName = searchParams.get('topic');

  useCapabilityFetch(capability, reloadTopics);

  usePageHeader(
    topicName !== null
      ? [
          { label: t(route('/video').labelKey), to: '/video' },
          { label: topicName, mono: true },
        ]
      : [],
  );

  return (
    <div className="p-6 h-full flex flex-col">
      <div className="flex-1 min-h-0">
        <CapabilityGate capability={capability} noCapability={t('video.noCapability')}>
          {topicsLoading && topics.length === 0 ? (
            <LoadingState variant="centered" />
          ) : topicsError !== null ? (
            <ErrorState
              message={apiErrorMessage(topicsError)}
              onRetry={() => void reloadTopics()}
            />
          ) : (
            <div className={cn(pageGrid, 'grid-cols-1 xl:grid-cols-[320px_minmax(0,1fr)]')}>
              <div className="min-h-0 overflow-y-auto rounded-panel border border-border bg-card p-2 h-[30vh] xl:h-full">
                <VideoTopicList />
              </div>
              <div className={cn(panelFill, 'relative rounded-panel border border-border overflow-hidden bg-card')}>
                {topicName === null ? (
                  <div className="h-full flex items-center justify-center">
                    <EmptyState message={t('video.selectHint')} />
                  </div>
                ) : (
                  <MseVideoStream key={topicName} topicName={topicName} />
                )}
              </div>
            </div>
          )}
        </CapabilityGate>
      </div>
    </div>
  );
}
