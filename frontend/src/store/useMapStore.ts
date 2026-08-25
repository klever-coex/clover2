import { create } from 'zustand';

import { createMapSelectionSlice } from './slices/mapSelectionSlice.ts';
import type { MapSelectionSlice } from './slices/mapSelectionSlice.ts';
import { createMapSlice } from './slices/mapSlice.ts';
import type { MapSlice } from './slices/mapSlice.ts';
import { createMarkerUiSlice } from './slices/markerUiSlice.ts';
import type { MarkerUiSlice } from './slices/markerUiSlice.ts';

export type MapStore = MapSlice & MarkerUiSlice & MapSelectionSlice;

/** Backend-first map page state. Not persisted — exprs/visible are session-only. */
export const useMapStore = create<MapStore>()((...args) => ({
  ...createMapSlice(...args),
  ...createMarkerUiSlice(...args),
  ...createMapSelectionSlice(...args),
}));
