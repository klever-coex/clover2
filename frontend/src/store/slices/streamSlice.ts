import type { StateCreator } from 'zustand';
import { clover2Api } from '../../api/clover2.ts';
import type { TopicSubscription } from '../../api/clover2.ts';
import { STREAM_BUFFER_CAP } from '../../constants/ros.ts';
import type { ApiError } from '@/types/errors';
import type { RosJsonValue } from '@/types/stream';
import type { RosStore } from '../useRosStore.ts';

export type StreamState = 'idle' | 'connecting' | 'connected' | 'closed' | 'error';

export interface StreamSlice {
  streamTopic: string | null;
  streamState: StreamState;
  streamError: ApiError | null;
  streamMessages: RosJsonValue[];
  streamReceived: number;
  subscribeTopic: (topicName: string) => void;
  closeStream: () => void;
  clearMessages: () => void;
  retryStream: () => void;
}

export const createStreamSlice: StateCreator<RosStore, [], [], StreamSlice> = (set, get) => {
  let subscription: TopicSubscription | null = null;

  return {
    streamTopic: null,
    streamState: 'idle',
    streamError: null,
    streamMessages: [],
    streamReceived: 0,

    subscribeTopic: (topicName) => {
      const prev = get();
      if (topicName === prev.streamTopic && prev.streamState === 'connected') return;

      subscription?.close();
      const sameTopic = topicName === prev.streamTopic;

      set({
        streamTopic: topicName,
        streamState: 'connecting',
        streamError: null,
        streamMessages: sameTopic ? prev.streamMessages : [],
        streamReceived: sameTopic ? prev.streamReceived : 0,
      });

      subscription = clover2Api.topics.subscribe(topicName, {
        onMessage: (message) => {
          if (get().streamTopic !== topicName) return;
          set((state) => ({
            streamState: 'connected',
            streamMessages: [...state.streamMessages, message].slice(-STREAM_BUFFER_CAP),
            streamReceived: state.streamReceived + 1,
          }));
        },
        onError: (error) => {
          if (get().streamTopic !== topicName) return;
          subscription = null;
          set({ streamState: 'error', streamError: error });
        },
        onClose: () => {
          if (get().streamTopic !== topicName) return;
          subscription = null;
          set({ streamState: 'closed' });
        },
      });
    },

    closeStream: () => {
      subscription?.close();
      subscription = null;
      set({ streamState: 'idle' });
    },

    clearMessages: () => {
      set({ streamMessages: [], streamReceived: 0 });
    },

    retryStream: () => {
      const { streamTopic } = get();
      if (streamTopic === null) return;
      get().subscribeTopic(streamTopic);
    },
  };
};
