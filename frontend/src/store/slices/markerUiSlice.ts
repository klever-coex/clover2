import type { StateCreator } from 'zustand';

import { DEFAULT_POSITION_EXPR, DEFAULT_ROTATION_EXPR } from '@/constants/defaults';
import { mToMm } from '@/constants/units';
import { rpyToEulerYxzDeg } from '../../utils/transformUtils.ts';
import type { MapMarker, MarkerUi } from '@/types/marker';
import type { MapStore } from '../useMapStore.ts';

export interface MarkerUiSlice {
  /** Session-only editing layer; seeded from pose on first sight, never refetched. */
  markerUi: Record<string, MarkerUi>;
  setExpr: (id: string, axis: 0 | 1 | 2, kind: 'position' | 'rotation', value: string) => void;
  seedUiFor: (markers: Record<string, MapMarker>) => void;
}

function uiFromMarker(m: MapMarker): MarkerUi {
  if (m.pose !== null) {
    const [rx, ry, rz] = rpyToEulerYxzDeg(m.pose.roll, m.pose.pitch, m.pose.yaw);
    return {
      positionExpr: [String(mToMm(m.pose.x)), String(mToMm(m.pose.y)), String(mToMm(m.pose.z))],
      rotationExpr: [String(rx), String(ry), String(rz)],
    };
  }
  return {
    positionExpr: [...DEFAULT_POSITION_EXPR],
    rotationExpr: [...DEFAULT_ROTATION_EXPR],
  };
}

export const createMarkerUiSlice: StateCreator<MapStore, [], [], MarkerUiSlice> = (set) => ({
  markerUi: {},

  setExpr: (id, axis, kind, value) =>
    set((s) => {
      const ui = s.markerUi[id];
      if (!ui) return s;
      const key = kind === 'position' ? 'positionExpr' : 'rotationExpr';
      const expr = [...ui[key]] as [string, string, string];
      expr[axis] = value;
      return { markerUi: { ...s.markerUi, [id]: { ...ui, [key]: expr } } };
    }),

  seedUiFor: (markers) =>
    set((s) => {
      const next: Record<string, MarkerUi> = { ...s.markerUi };
      for (const [id, m] of Object.entries(markers)) {
        if (!(id in next)) next[id] = uiFromMarker(m); // seed once; never clobber edits
      }
      return { markerUi: next };
    }),
});
