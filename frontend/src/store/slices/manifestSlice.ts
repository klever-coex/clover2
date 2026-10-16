import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import type { Capability, Manifest } from '@/types/manifest';
import { createResourceSlice, type ResourceSlice } from '../resourceSlice.ts';
import type { RosStore } from '../useRosStore.ts';

export type ManifestSlice = ResourceSlice<'manifest', Manifest | null> & {
  hasCapability: (capability: Capability) => boolean;
};

export const createManifestSlice: StateCreator<RosStore, [], [], ManifestSlice> = (
  set,
  get,
  api,
) => ({
  ...createResourceSlice<RosStore, 'manifest', Manifest | null>({
    name: 'manifest',
    initial: null,
    fetcher: () => clover2Api.manifest.get(),
  })(set, get, api),

  hasCapability: (capability) =>
    get().manifest?.plugins.some((plugin) =>
      plugin.capabilities.includes(capability),
    ) ?? false,
});
