import { create } from 'zustand';
import { createDiagnosticsSlice, type DiagnosticsSlice } from './slices/diagnosticsSlice.ts';
import { createLogsSlice, type LogsSlice } from './slices/logsSlice.ts';
import { createManifestSlice, type ManifestSlice } from './slices/manifestSlice.ts';
import { createNodesSlice, type NodesSlice } from './slices/nodesSlice.ts';
import { createStreamSlice, type StreamSlice } from './slices/streamSlice.ts';
import { createTopicsSlice, type TopicsSlice } from './slices/topicsSlice.ts';

export type RosStore =
  & ManifestSlice
  & TopicsSlice
  & StreamSlice
  & NodesSlice
  & LogsSlice
  & DiagnosticsSlice;

export const useRosStore = create<RosStore>()((...args) => ({
  ...createManifestSlice(...args),
  ...createTopicsSlice(...args),
  ...createStreamSlice(...args),
  ...createNodesSlice(...args),
  ...createLogsSlice(...args),
  ...createDiagnosticsSlice(...args),
}));

export {
  type ManifestSlice,
  type TopicsSlice,
  type StreamSlice,
  type NodesSlice,
  type LogsSlice,
  type DiagnosticsSlice,
};
