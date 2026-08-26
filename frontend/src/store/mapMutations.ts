import i18n from '../i18n/index.ts';
import { clover2Api } from '../api/clover2.ts';
import { ApiError } from '../types/errors.ts';
import { DEFAULT_MARKER_SIZE_M } from '../constants/defaults.ts';
import type { MarkerInfo, ModifyResult } from '../types/map.ts';
import type { MapMarker } from '../types/marker.ts';
import { resolvePose } from '../utils/transformUtils.ts';
import { mapDirtyIds } from './slices/mapSlice.ts';
import { useMapStore } from './useMapStore.ts';

export function validateSize(sizeM: number): string | null {
  if (!Number.isFinite(sizeM)) return i18n.t('map.sizeFinite');
  if (sizeM <= 0) return i18n.t('map.sizePositive');
  if (sizeM < 0.01) return i18n.t('map.sizeMin');
  return null;
}

function markerToInfo(m: MapMarker, ui: { positionExpr: [string, string, string]; rotationExpr: [string, string, string] } | undefined): MarkerInfo {
  const fallback = m.pose ?? { x: 0, y: 0, z: 0.01, roll: 0, pitch: 0, yaw: 0 };
  const pose = ui ? resolvePose(ui.positionExpr, ui.rotationExpr, m.id) : fallback;
  return { id: m.id, type: m.type, size: m.sizeM, marker_frame_id: m.markerFrameId, pose };
}

export function addMarker(): void {
  const s = useMapStore.getState();
  const knownIds = [
    ...Object.values(s.markers).map((m) => m.id),
    ...Object.keys(s.baseline).map(Number),
  ];
  const id = (knownIds.length ? Math.max(...knownIds) : -1) + 1;
  const info: MarkerInfo = {
    id,
    type: 'fixed',
    size: DEFAULT_MARKER_SIZE_M,
    marker_frame_id: `Marker ${String(id).padStart(2, '0')}`,
    pose: {
      x: (Object.keys(s.markers).length * 0.15) % 3,
      y: Math.floor(Object.keys(s.markers).length / 5) * 0.15,
      z: 0.01,
      roll: 0,
      pitch: 0,
      yaw: 0,
    },
  };

  s.addMarkerLocal(info);
  s.seedUiFor({ [String(id)]: { id, type: 'fixed', sizeM: info.size, markerFrameId: info.marker_frame_id, pose: info.pose ?? null } });
  s.selectMarker(String(id));
}

/** Local-only: removes markers; committed to the backend by saveMap(). */
export function deleteMarkers(ids: string[]): void {
  const s = useMapStore.getState();
  s.setMutationError(null);
  ids.forEach((id) => s.removeMarkerLocal(id));
  s.deselectAll();
}

/** Pushes every local change (edits, adds, deletes) to the backend, then refetches. */
export async function saveMap(): Promise<void> {
  const s = useMapStore.getState();
  if (s.saving) return;

  const ops: Array<() => Promise<ModifyResult>> = [];
  for (const id of mapDirtyIds(s)) {
    const marker = s.markers[id];
    if (marker === undefined) {
      const numericId = Number(id);
      ops.push(() => clover2Api.map.delete(numericId));
      continue;
    }
    const info = markerToInfo(marker, s.markerUi[id]);
    ops.push(() =>
      id in s.baseline ? clover2Api.map.edit(marker.id, info) : clover2Api.map.add(info),
    );
  }
  if (ops.length === 0) return;

  s.setSaving(true);
  s.setMutationError(null);
  try {
    for (const op of ops) {
      const result = await op();
      if (!result.success) {
        throw new ApiError(result.error_message || i18n.t('common.unknownError'));
      }
    }
  } catch (error) {
    s.setMutationError(error instanceof ApiError ? error.message : String(error));
    s.setSaving(false);
    return;
  }
  s.setSaving(false);
  await s.reloadMap();
}

