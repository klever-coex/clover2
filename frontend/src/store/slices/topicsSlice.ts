import type { StateCreator } from 'zustand';
import { clover2Api } from '../../api/clover2.ts';
import { toApiError } from '../../types/ros.ts';
import type { ApiError, TopicInfo } from '../../types/ros.ts';
import type { RosStore } from '../useRosStore.ts';

export interface TopicsSlice {
  topics: TopicInfo[];
  topicsLoading: boolean;
  topicsError: ApiError | null;
  reloadTopics: () => Promise<void>;
}

export const createTopicsSlice: StateCreator<RosStore, [], [], TopicsSlice> = (set, get) => ({
  topics: [],
  topicsLoading: false,
  topicsError: null,

  reloadTopics: async () => {
    if (get().topicsLoading) return;
    set({ topicsLoading: true, topicsError: null });
    try {
      const topics = await clover2Api.getTopics();
      set({ topics, topicsLoading: false });
    } catch (error) {
      set({ topicsLoading: false, topicsError: toApiError(error) });
    }
  },
});
