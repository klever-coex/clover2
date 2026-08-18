import type { StateCreator } from 'zustand';
import { clover2Api } from '../../api/clover2.ts';
import { toApiError } from '../../types/ros.ts';
import type { ApiError, Capability, Manifest } from '../../types/ros.ts';
import type { RosStore } from '../useRosStore.ts';

export interface ManifestSlice {
  manifest: Manifest | null;
  manifestLoading: boolean;
  manifestError: ApiError | null;
  hasCapability: (capability: Capability) => boolean;
  fetchManifest: () => Promise<void>;
}

export const createManifestSlice: StateCreator<RosStore, [], [], ManifestSlice> = (set, get) => ({
  manifest: null,
  manifestLoading: false,
  manifestError: null,

  hasCapability: (capability) => {
    const { manifest } = get();
    return manifest?.plugins.some((plugin) => plugin.capabilities.includes(capability)) ?? false;
  },

  fetchManifest: async () => {
    if (get().manifestLoading) return;
    set({ manifestLoading: true, manifestError: null });
    try {
      // Drop the cached manifest first: after a backend restart the capabilities may change.
      clover2Api.clearManifestCache();
      const manifest = await clover2Api.getManifest();
      set({ manifest, manifestLoading: false });
    } catch (error) {
      set({ manifestLoading: false, manifestError: toApiError(error) });
    }
  },
});
