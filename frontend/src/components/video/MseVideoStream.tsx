import { useRef } from 'react';
import { useTranslation } from 'react-i18next';

import { useMseStream } from '@/hooks/useMseStream';
import { ErrorState } from '../common/ErrorState.tsx';
import { Spinner } from '@/components/ui/spinner';

interface MseVideoStreamProps {
  topicName: string;
}

export function MseVideoStream({ topicName }: MseVideoStreamProps) {
  const { t } = useTranslation();
  const videoRef = useRef<HTMLVideoElement>(null);
  const { status, retry } = useMseStream(topicName, videoRef);

  if (status === 'error') {
    return (
      <div className="h-full flex items-center justify-center p-6">
        <ErrorState message={t('video.streamError')} onRetry={retry} />
      </div>
    );
  }

  return (
    <div className="relative size-full bg-background">
      <video
        ref={videoRef}
        autoPlay
        muted
        playsInline
        className="absolute inset-0 size-full object-contain"
      />
      {status === 'loading' && (
        <div className="absolute inset-0 flex items-center justify-center bg-background/60">
          <Spinner className="size-8 text-primary" />
        </div>
      )}
    </div>
  );
}
