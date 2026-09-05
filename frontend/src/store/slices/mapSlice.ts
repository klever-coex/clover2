import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import { DEFAULT_DICTIONARY } from '@/constants/defaults';
import { isDictionaryName } from '../../data/dictionaries/index.ts';
import { toApiError } from '@/types/errors';
import type { ApiError } from '@/types/errors';
import type { MarkerInfo } from '@/types/map';
import type { ArUcoDictionary, MapMarker, MarkerType } from '@/types/marker';
import type { MapStore } from '../useMapStore.ts';

export interface MapBaselineEntry {
  markerFrameId: string;
  sizeM: number;
  positionExpr: readonly [string, string, string];
  rotationExpr: readonly [string, string, string];
}

export interface MapSlice {
  markers: Record<string, MapMarker>;
  mapMeta: { dictionary: ArUcoDictionary; frameId: string; name: string } | null;
  mapLoading: boolean;
  mapError: ApiError | null;
  mutationError: string | null;
  baseline: Record<string, MapBaselineEntry>;
  saving: boolean;
  setMutationError: (message: string | null) => void;
  setSaving: (saving: boolean) => void;
  captureBaseline: () => void;
  reloadMap: () => Promise<void>;
  setMarkerInfoLocal: (id: string, patch: Partial<Pick<MapMarker, 'sizeM' | 'markerFrameId'>>) => void;
  addMarkerLocal: (info: MarkerInfo) => void;
  removeMarkerLocal: (id: string) => void;
  pruneSelection: () => void;
}

export function mapDirtyIds(state: MapStore): string[] {
  const dirty: string[] = [];

  for (const [id, marker] of Object.entries(state.markers)) {
    const base = state.baseline[id];
    if (base === undefined) {
      dirty.push(id); // added locally, not yet saved
      continue;
    }
    const ui = state.markerUi[id];
    const exprsChanged =
      ui !== undefined &&
      (ui.positionExpr[0] !== base.positionExpr[0] ||
        ui.positionExpr[1] !== base.positionExpr[1] ||
        ui.positionExpr[2] !== base.positionExpr[2] ||
        ui.rotationExpr[0] !== base.rotationExpr[0] ||
        ui.rotationExpr[1] !== base.rotationExpr[1] ||
        ui.rotationExpr[2] !== base.rotationExpr[2]);
    if (
      marker.markerFrameId !== base.markerFrameId ||
      marker.sizeM !== base.sizeM ||
      exprsChanged
    ) {
      dirty.push(id);
    }
  }

  for (const id of Object.keys(state.baseline)) {
    if (!(id in state.markers)) dirty.push(id);
  }

  return dirty;
}

export const createMapSlice: StateCreator<MapStore, [], [], MapSlice> = (set, get) => ({
  markers: {},
  mapMeta: null,
  mapLoading: false,
  mapError: null,
  mutationError: null,
  baseline: {},
  saving: false,

  setMutationError: (message) => set({ mutationError: message }),

  setSaving: (saving) => set({ saving }),

  captureBaseline: () =>
    set((s) => {
      const baseline: Record<string, MapBaselineEntry> = {};
      for (const [id, marker] of Object.entries(s.markers)) {
        const ui = s.markerUi[id];
        const positionExpr: [string, string, string] = ui ? [...ui.positionExpr] : ['0', '0', '0'];
        const rotationExpr: [string, string, string] = ui ? [...ui.rotationExpr] : ['0', '0', '0'];
        baseline[id] = {
          markerFrameId: marker.markerFrameId,
          sizeM: marker.sizeM,
          positionExpr,
          rotationExpr,
        };
      }
      return { baseline };
    }),

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
      get().captureBaseline(); // everything loaded is considered saved
    } catch (error) {
      set({ mapLoading: false, mapError: toApiError(error) });
    }
  },

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
