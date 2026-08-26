import { useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { VIDEO_VP8_MIME } from '../../constants/ros.ts';
import { videoStreamUrl } from '../../utils/videoStream.ts';
import { ErrorState } from '../ui/ErrorState.tsx';
import { Spinner } from '../ui/Spinner.tsx';

interface H264VideoStreamProps {
  topicName: string;
}

const STALL_TIMEOUT_MS = 10_000;

export function H264VideoStream({ topicName }: H264VideoStreamProps) {
  const { t } = useTranslation();
  const [status, setStatus] = useState<'loading' | 'playing' | 'error'>('loading');
  const [attempt, setAttempt] = useState(0);
  const videoRef = useRef<HTMLVideoElement>(null);

  useEffect(() => {
    let aborted = false;
    let objectUrl: string | null = null;
    let sourceBuffer: SourceBuffer | null = null;
    let reader: ReadableStreamDefaultReader<Uint8Array<ArrayBuffer>> | null = null;
    let watchdog: number | null = null;
    const mediaSource = new MediaSource();

    setStatus('loading');

    const clearWatchdog = () => {
      if (watchdog !== null) window.clearTimeout(watchdog);
    };
    const resetWatchdog = () => {
      clearWatchdog();
      watchdog = window.setTimeout(() => {
        if (!aborted) setStatus('error');
      }, STALL_TIMEOUT_MS);
    };

    const video = videoRef.current;
    if (video === null) return;

    const fail = () => {
      clearWatchdog();
      if (!aborted) setStatus('error');
    };

    const onSourceOpen = () => {
      if (aborted) return;

      try {
        sourceBuffer = mediaSource.addSourceBuffer(VIDEO_VP8_MIME);
      } catch {
        fail();
        return;
      }

      void (async () => {
        try {
          const response = await fetch(videoStreamUrl(topicName, 'vp8'));
          if (!response.ok || response.body === null) {
            throw new Error(`HTTP ${response.status}`);
          }
          reader = response.body.getReader();
          resetWatchdog();

          while (!aborted) {
            const { done, value } = await reader.read();
            if (done) throw new Error('Stream ended');
            if (aborted || value === undefined) break;
            resetWatchdog();

            const sb = sourceBuffer;
            if (sb === null || mediaSource.readyState !== 'open') continue;

            if (sb.updating) {
              await new Promise<void>((resolve) => {
                if (aborted) {
                  resolve();
                  return;
                }
                const onEnded = () => {
                  sb.removeEventListener('updateend', onEnded);
                  resolve();
                };
                sb.addEventListener('updateend', onEnded);
              });
              if (aborted) break;
            }

            sb.appendBuffer(value);
            setStatus((current) => (current === 'error' ? current : 'playing'));

            if (sb.buffered.length > 0) {
              const start = sb.buffered.start(0);
              const end = sb.buffered.end(sb.buffered.length - 1);
              if (end - start > 60 && !sb.updating) {
                try {
                  sb.remove(start, end - 30);
                } catch {
                }
              }
            }
          }
        } catch {
          fail();
        }
      })();
    };

    mediaSource.addEventListener('sourceopen', onSourceOpen);

    if (typeof MediaSource === 'undefined' || !MediaSource.isTypeSupported(VIDEO_VP8_MIME)) {
      setStatus('error');
      return;
    }

    objectUrl = URL.createObjectURL(mediaSource);
    video.src = objectUrl;
    void video.play().catch(() => {});

    return () => {
      aborted = true;
      clearWatchdog();
      mediaSource.removeEventListener('sourceopen', onSourceOpen);
      void reader?.cancel().catch(() => {});
      reader = null;
      if (sourceBuffer !== null) {
        try {
          if (!sourceBuffer.updating) mediaSource.removeSourceBuffer(sourceBuffer);
        } catch {
        }
        sourceBuffer = null;
      }
      try {
        if (mediaSource.readyState === 'open') mediaSource.endOfStream();
      } catch {
      }
      if (objectUrl !== null) URL.revokeObjectURL(objectUrl);
    };
  }, [topicName, attempt]);

  if (status === 'error') {
    return (
      <div className="h-full flex items-center justify-center p-6">
        <ErrorState
          message={t('video.streamError')}
          onRetry={() => {
            setStatus('loading');
            setAttempt((current) => current + 1);
          }}
        />
      </div>
    );
  }

  return (
    <div className="relative w-full h-full bg-surface-0">
      <video ref={videoRef} autoPlay muted playsInline className="absolute inset-0 w-full h-full object-contain" />
      {status === 'loading' && (
        <div className="absolute inset-0 flex items-center justify-center bg-surface-0/60">
          <Spinner size={32} />
        </div>
      )}
    </div>
  );
}
