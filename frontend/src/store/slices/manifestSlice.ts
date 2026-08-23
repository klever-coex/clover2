import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import type { Capability, Manifest } from '../../types/manifest.ts';
import { createResourceSlice, type ResourceSlice } from '../middleware/resourceSlice.ts';
import type { RosStore } from '../useRosStore.ts';

export type ManifestSlice = ResourceSlice<'manifest', Manifest | null, 'fetchManifest'> & {
  hasCapability: (capability: Capability) => boolean;
};

export const createManifestSlice: StateCreator<RosStore, [], [], ManifestSlice> = (
  set,
  get,
  api,
) => ({
  ...createResourceSlice<RosStore, 'manifest', Manifest | null, 'fetchManifest'>({
    name: 'manifest',
    initial: null,
    fetch: 'fetchManifest',
    fetcher: () => {
      // Drop the cached manifest first: after a backend restart the capabilities may change.
      clover2Api.manifest.clearCache();
      return clover2Api.manifest.get();
    },
  })(set, get, api),

  hasCapability: (capability) =>
    get().manifest?.plugins.some((plugin) =>
      plugin.capabilities.includes(capability),
    ) ?? false,
});
