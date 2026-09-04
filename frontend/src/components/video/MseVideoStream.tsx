import { useRef } from 'react';
import { useTranslation } from 'react-i18next';

import { useMseStream } from '../../hooks/useMseStream.ts';
import { ErrorState } from '../ui/ErrorState.tsx';
import { Spinner } from '../ui/Spinner.tsx';

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
    <div className="relative w-full h-full bg-surface-0">
      <video
        ref={videoRef}
        autoPlay
        muted
        playsInline
        className="absolute inset-0 w-full h-full object-contain"
      />
      {status === 'loading' && (
        <div className="absolute inset-0 flex items-center justify-center bg-surface-0/60">
          <Spinner size={32} />
        </div>
      )}
    </div>
  );
}
