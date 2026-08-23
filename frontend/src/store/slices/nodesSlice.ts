import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import { createResourceSlice, type ResourceSlice } from '../middleware/resourceSlice.ts';
import type { RosStore } from '../useRosStore.ts';

export type NodesSlice = ResourceSlice<'nodes', string[]>;

export const createNodesSlice: StateCreator<RosStore, [], [], NodesSlice> =
  createResourceSlice<RosStore, 'nodes', string[]>({
    name: 'nodes',
    initial: [],
    fetcher: () => clover2Api.nodes.list(),
  });
