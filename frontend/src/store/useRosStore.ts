import { create } from 'zustand';
import { createManifestSlice, type ManifestSlice } from './slices/manifestSlice.ts';
import { createStreamSlice, type StreamSlice } from './slices/streamSlice.ts';
import { createTopicsSlice, type TopicsSlice } from './slices/topicsSlice.ts';

export type RosStore = ManifestSlice & TopicsSlice & StreamSlice;

export const useRosStore = create<RosStore>()((...args) => ({
  ...createManifestSlice(...args),
  ...createTopicsSlice(...args),
  ...createStreamSlice(...args),
}));

export { type ManifestSlice, type TopicsSlice, type StreamSlice };
