import type { StateCreator } from 'zustand';
import { clover2Api } from '../../api/clover2.ts';
import type { TopicSubscription } from '../../api/clover2.ts';
import { STREAM_BUFFER_CAP } from '../../constants/ros.ts';
import type { ApiError, RosJsonValue } from '../../types/ros.ts';
import type { RosStore } from '../useRosStore.ts';

export type StreamState = 'idle' | 'connecting' | 'connected' | 'closed' | 'error';

export interface StreamSlice {
  streamTopic: string | null;
  streamState: StreamState;
  streamError: ApiError | null;
  streamMessages: RosJsonValue[];
  /** Total frames delivered since the last subscribe; the only client-side signal of backend rate limiting. */
  streamReceived: number;
  streamSubscription: TopicSubscription | null;
  subscribeTopic: (topicName: string) => void;
  closeStream: () => void;
  clearMessages: () => void;
  retryStream: () => void;
}

export const createStreamSlice: StateCreator<RosStore, [], [], StreamSlice> = (set, get) => ({
  streamTopic: null,
  streamState: 'idle',
  streamError: null,
  streamMessages: [],
  streamReceived: 0,
  streamSubscription: null,

  subscribeTopic: (topicName) => {
    const prev = get();
    if (topicName === prev.streamTopic && prev.streamState === 'connected') return;

    // Tear down any active subscription before switching topics.
    prev.streamSubscription?.close();
    const sameTopic = topicName === prev.streamTopic;

    set({
      streamTopic: topicName,
      streamState: 'connecting',
      streamError: null,
      // Resubscribing to the same topic (Retry) keeps the buffer; a new topic starts fresh.
      streamMessages: sameTopic ? prev.streamMessages : [],
      streamReceived: sameTopic ? prev.streamReceived : 0,
    });

    const subscription = clover2Api.subscribeTopic(topicName, {
      onMessage: (message) => {
        if (get().streamTopic !== topicName) return; // Stale subscription guard.
        set((state) => ({
          streamState: 'connected',
          streamMessages: [...state.streamMessages, message].slice(-STREAM_BUFFER_CAP),
          streamReceived: state.streamReceived + 1,
        }));
      },
      onError: (error) => {
        if (get().streamTopic !== topicName) return;
        set({ streamState: 'error', streamError: error, streamSubscription: null });
      },
      onClose: () => {
        if (get().streamTopic !== topicName) return;
        set({ streamState: 'closed', streamSubscription: null });
      },
    });

    set({ streamSubscription: subscription });
  },

  closeStream: () => {
    get().streamSubscription?.close();
    set({ streamSubscription: null, streamState: 'idle' });
  },

  clearMessages: () => {
    set({ streamMessages: [] });
  },

  retryStream: () => {
    const { streamTopic } = get();
    if (streamTopic === null) return;
    get().subscribeTopic(streamTopic);
  },
});
