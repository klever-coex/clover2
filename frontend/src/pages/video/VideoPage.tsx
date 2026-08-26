import { useTranslation } from 'react-i18next';
import { useSearchParams } from 'react-router';

import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { MseVideoStream } from '../../components/video/MseVideoStream.tsx';
import { VideoTopicList } from '../../components/video/VideoTopicList.tsx';
import { EmptyState } from '../../components/ui/EmptyState.tsx';
import { ErrorState } from '../../components/ui/ErrorState.tsx';
import { LoadingState } from '../../components/ui/LoadingState.tsx';
import { PageHeader } from '../../components/ui/PageHeader.tsx';
import { useApiErrorMessage } from '../../hooks/useApiErrorMessage.ts';
import { useCapabilityFetch } from '../../hooks/useCapabilityFetch.ts';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useRosStore } from '../../store/useRosStore.ts';

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

  return (
    <div className="p-6 h-full flex flex-col">
      <PageHeader
        title={
          topicName !== null ? (
            <span className="font-mono">{topicName}</span>
          ) : (
            t('video.title')
          )
        }
      />

      <div className="mt-4 flex-1 min-h-0">
        <CapabilityGate capability={capability} noCapability={t('video.noCapability')}>
          {topicsLoading && topics.length === 0 ? (
            <LoadingState variant="centered" />
          ) : topicsError !== null ? (
            <ErrorState
              message={apiErrorMessage(topicsError)}
              onRetry={() => void reloadTopics()}
            />
          ) : (
            <div className="grid gap-3 grid-cols-1 xl:grid-cols-[320px_minmax(0,1fr)] h-full min-h-[420px]">
              <div className="min-h-0 overflow-y-auto rounded-panel border border-line bg-surface-1 p-2 h-[30vh] xl:h-full">
                <VideoTopicList />
              </div>
              <div className="relative rounded-panel border border-line overflow-hidden bg-surface-1 h-[60vh] xl:h-full min-h-0">
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
