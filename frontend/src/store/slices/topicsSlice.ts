import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import type { TopicInfo } from '../../types/topic.ts';
import { createResourceSlice, type ResourceSlice } from '../middleware/resourceSlice.ts';
import type { RosStore } from '../useRosStore.ts';

export type TopicsSlice = ResourceSlice<'topics', TopicInfo[]>;

export const createTopicsSlice: StateCreator<RosStore, [], [], TopicsSlice> =
  createResourceSlice<RosStore, 'topics', TopicInfo[]>({
    name: 'topics',
    initial: [],
    fetcher: () => clover2Api.topics.list(),
  });
