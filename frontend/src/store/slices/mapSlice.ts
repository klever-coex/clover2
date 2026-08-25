import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import { DEFAULT_DICTIONARY } from '../../constants/defaults.ts';
import { isDictionaryName } from '../../data/dictionaries/index.ts';
import { toApiError } from '../../types/errors.ts';
import type { ApiError } from '../../types/errors.ts';
import type { MarkerInfo, MarkerPose } from '../../types/map.ts';
import type { ArUcoDictionary, MapMarker, MarkerType } from '../../types/marker.ts';
import type { MapStore } from '../useMapStore.ts';

export interface MapSlice {
  markers: Record<string, MapMarker>;
  mapMeta: { dictionary: ArUcoDictionary; frameId: string; name: string } | null;
  mapLoading: boolean;
  mapError: ApiError | null;
  mutationError: string | null;
  setMutationError: (message: string | null) => void;
  reloadMap: () => Promise<void>;
  setMarkerPoseLocal: (id: string, pose: MarkerPose) => void;
  setMarkerInfoLocal: (id: string, patch: Partial<Pick<MapMarker, 'sizeM' | 'markerFrameId'>>) => void;
  addMarkerLocal: (info: MarkerInfo) => void;
  removeMarkerLocal: (id: string) => void;
  pruneSelection: () => void;
}

export const createMapSlice: StateCreator<MapStore, [], [], MapSlice> = (set, get) => ({
  markers: {},
  mapMeta: null,
  mapLoading: false,
  mapError: null,
  mutationError: null,

  setMutationError: (message) => set({ mutationError: message }),

  reloadMap: async () => {
    if (get().mapLoading) return; // dedup, like createResourceSlice
    set({ mapLoading: true, mapError: null });
    try {
      const info = await clover2Api.map.get();
      const markers: Record<string, MapMarker> = {};
      for (const m of info.markers ?? []) {
        markers[String(m.id)] = {
          id: m.id,
          type: m.type as MarkerType,
          sizeM: m.size,
          markerFrameId: m.marker_frame_id,
          pose: m.pose ?? null,
        };
      }
      const dictionary = isDictionaryName(info.dictionary)
        ? info.dictionary
        : DEFAULT_DICTIONARY;
      set({
        markers,
        mapLoading: false,
        mapError: null,
        mapMeta: { dictionary, frameId: info.frame_id ?? '', name: info.name ?? '' },
      });
      get().seedUiFor(markers); // cross-slice: seed exprs only for unseen markers
      get().pruneSelection(); // drop selection for deleted markers
    } catch (error) {
      set({ mapLoading: false, mapError: toApiError(error) });
    }
  },

  setMarkerPoseLocal: (id, pose) =>
    set((s) => {
      const m = s.markers[id];
      if (!m || m.pose === null) return s;
      return { markers: { ...s.markers, [id]: { ...m, pose } } };
    }),

  setMarkerInfoLocal: (id, patch) =>
    set((s) => {
      const m = s.markers[id];
      if (!m) return s;
      return { markers: { ...s.markers, [id]: { ...m, ...patch } } };
    }),

  addMarkerLocal: (info) =>
    set((s) => ({
      markers: {
        ...s.markers,
        [String(info.id)]: {
          id: info.id,
          type: info.type as MarkerType,
          sizeM: info.size,
          markerFrameId: info.marker_frame_id,
          pose: info.pose ?? null,
        },
      },
    })),

  removeMarkerLocal: (id) =>
    set((s) => {
      const { [id]: _removed, ...rest } = s.markers;
      return { markers: rest };
    }),

  pruneSelection: () =>
    set((s) => ({
      selectedMarkerIds: s.selectedMarkerIds.filter((id) => id in s.markers),
    })),
});
