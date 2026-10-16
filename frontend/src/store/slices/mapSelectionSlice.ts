import type { StateCreator } from 'zustand';

import type { MapStore } from '../useMapStore.ts';

export interface MapSelectionSlice {
  selectedMarkerIds: string[];
  selectMarker: (id: string | null, additive?: boolean) => void;
  deselectAll: () => void;
  selectAll: () => void;
}

export const createMapSelectionSlice: StateCreator<MapStore, [], [], MapSelectionSlice> = (
  set,
) => ({
  selectedMarkerIds: [],

  selectMarker: (id, additive) => {
    set((state) => {
      if (id === null) return { selectedMarkerIds: [] };
      if (additive) {
        const exists = state.selectedMarkerIds.includes(id);
        return {
          selectedMarkerIds: exists
            ? state.selectedMarkerIds.filter((sid) => sid !== id)
            : [...state.selectedMarkerIds, id],
        };
      }
      return { selectedMarkerIds: [id] };
    });
  },

  deselectAll: () => set({ selectedMarkerIds: [] }),
  selectAll: () => set((state) => ({ selectedMarkerIds: Object.keys(state.markers) })),
});
