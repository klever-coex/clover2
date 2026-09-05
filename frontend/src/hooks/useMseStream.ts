import { useEffect, useState } from 'react';
import type { RefObject } from 'react';

import { VIDEO_VP8_MIME } from '../constants/ros.ts';
import { videoStreamUrl } from '../utils/videoStream.ts';

const STALL_TIMEOUT_MS = 10_000;
const BUFFER_TRIM_THRESHOLD_S = 60;
const BUFFER_KEEP_TAIL_S = 30;

export type MseStreamStatus = 'loading' | 'playing' | 'error';

export function useMseStream(
  topicName: string,
  videoRef: RefObject<HTMLVideoElement | null>,
): { status: MseStreamStatus; retry: () => void } {
  const [status, setStatus] = useState<MseStreamStatus>('loading');
  const [attempt, setAttempt] = useState(0);

  const mseSupported =
    typeof MediaSource !== 'undefined' && MediaSource.isTypeSupported(VIDEO_VP8_MIME);

  useEffect(() => {
    if (!mseSupported) return;

    let aborted = false;
    let objectUrl: string | null = null;
    let sourceBuffer: SourceBuffer | null = null;
    let reader: ReadableStreamDefaultReader<Uint8Array<ArrayBuffer>> | null = null;
    let watchdog: number | null = null;
    let abortController: AbortController | null = null;
    const mediaSource = new MediaSource();

    const fail = () => {
      stopStreaming();
      if (!aborted) setStatus('error');
    };

    const resetWatchdog = () => {
      if (watchdog !== null) window.clearTimeout(watchdog);
      watchdog = window.setTimeout(() => {
        if (!aborted) fail();
      }, STALL_TIMEOUT_MS);
    };

    /** Cancels the in-flight fetch/reader so no work continues in background. */
    const stopStreaming = () => {
      if (watchdog !== null) window.clearTimeout(watchdog);
      abortController?.abort();
      void reader?.cancel().catch(() => {});
      reader = null;
    };

    const waitForUpdateEnd = (sb: SourceBuffer) =>
      new Promise<void>((resolve) => {
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

    /** Keeps the SourceBuffer from growing without bound on long streams. */
    const trimBuffer = (sb: SourceBuffer) => {
      if (sb.buffered.length === 0 || sb.updating) return;
      const start = sb.buffered.start(0);
      const end = sb.buffered.end(sb.buffered.length - 1);
      if (end - start <= BUFFER_TRIM_THRESHOLD_S) return;
      try {
        sb.remove(start, end - BUFFER_KEEP_TAIL_S);
      } catch {
        // remove() throws when called mid-update; it will retry on the next chunk
      }
    };

    const pump = async (sb: SourceBuffer) => {
      resetWatchdog();
      abortController = new AbortController();
      const response = await fetch(videoStreamUrl(topicName, 'vp8'), {
        signal: abortController.signal,
      });
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

        const current = sb;
        if (mediaSource.readyState !== 'open') continue;
        if (current.updating) await waitForUpdateEnd(current);
        if (aborted) break;

        current.appendBuffer(value);
        setStatus((s) => (s === 'error' ? s : 'playing'));
        trimBuffer(current);
      }
    };

    const onSourceOpen = () => {
      if (aborted) return;
      try {
        sourceBuffer = mediaSource.addSourceBuffer(VIDEO_VP8_MIME);
      } catch {
        fail();
        return;
      }
      pump(sourceBuffer).catch(fail);
    };

    mediaSource.addEventListener('sourceopen', onSourceOpen);

    const video = videoRef.current;
    if (video !== null) {
      objectUrl = URL.createObjectURL(mediaSource);
      video.src = objectUrl;
      void video.play().catch(() => {
        // Autoplay can be rejected before user interaction; muted retries usually succeed
      });
    }

    return () => {
      aborted = true;
      abortController?.abort();
      if (watchdog !== null) window.clearTimeout(watchdog);
      mediaSource.removeEventListener('sourceopen', onSourceOpen);
      void reader?.cancel().catch(() => {});
      reader = null;
      if (sourceBuffer !== null) {
        try {
          if (!sourceBuffer.updating) mediaSource.removeSourceBuffer(sourceBuffer);
        } catch {
          // SourceBuffer may already be gone along with the MediaSource
        }
        sourceBuffer = null;
      }
      try {
        if (mediaSource.readyState === 'open') mediaSource.endOfStream();
      } catch {
        // Same as above — teardown order is not guaranteed
      }
      if (objectUrl !== null) URL.revokeObjectURL(objectUrl);
    };
  }, [topicName, attempt, mseSupported, videoRef]);

  return {
    status: mseSupported ? status : 'error',
    retry: () => {
      setStatus('loading');
      setAttempt((current) => current + 1);
    },
  };
}
