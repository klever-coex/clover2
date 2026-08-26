import { DEFAULT_GRID_STEP_MM, DEFAULT_MARKER_SIZE_M, DEFAULT_POSITION_EXPR, DEFAULT_PROJECT_NAME, DEFAULT_ROTATION_EXPR } from '../../constants/defaults.ts';
import { useMapStore } from '../../store/useMapStore.ts';
import { clover2Api } from '../../api/clover2.ts';
import { ApiError } from '../../types/errors.ts';
import type { MarkerInfo, ModifyResult } from '../../types/map.ts';
import type { MapMarker, MarkerSnapshot, ProjectFile } from '../../types/marker.ts';
import { eulerDegToRotationExpr, resolvePose, rpyToEulerYxzDeg, vec3ToPositionExpr } from '../../utils/transformUtils.ts';

export function validateSize(sizeM: number): string | null {
  if (!Number.isFinite(sizeM)) return 'Size must be a finite number';
  if (sizeM <= 0) return 'Size must be positive';
  if (sizeM < 0.01) return 'Size must be at least 0.01m (10mm)';
  return null;
}

function markerToInfo(m: MapMarker, ui: { positionExpr: [string, string, string]; rotationExpr: [string, string, string] } | undefined): MarkerInfo {
  const fallback = m.pose ?? { x: 0, y: 0, z: 0.01, roll: 0, pitch: 0, yaw: 0 };
  const pose = ui ? resolvePose(ui.positionExpr, ui.rotationExpr, m.id) : fallback;
  return { id: m.id, type: m.type, size: m.sizeM, marker_frame_id: m.markerFrameId, pose };
}

async function run(
  op: () => Promise<ModifyResult>,
  local: () => void,
): Promise<{ ok: boolean; error: string | null }> {
  const s = useMapStore.getState();
  s.setMutationError(null);
  try {
    const result = await op();
    if (!result.success) {
      const message = result.error_message || 'Unknown error';
      s.setMutationError(message);
      return { ok: false, error: message };
    }
    local();
    void s.reloadMap(); // refetch backend truth after every mutation
    return { ok: true, error: null };
  } catch (error) {
    const message = error instanceof ApiError ? error.message : String(error);
    s.setMutationError(message);
    return { ok: false, error: message };
  }
}

export async function applyPose(id: string): Promise<{ ok: boolean; error: string | null }> {
  const s = useMapStore.getState();
  const marker = s.markers[id];
  if (!marker || marker.type !== 'fixed' || marker.pose === null) return { ok: true, error: null };

  const info = markerToInfo(marker, s.markerUi[id]);
  return run(
    () => clover2Api.map.edit(marker.id, info),
    () => {
      if (info.pose !== undefined) s.setMarkerPoseLocal(id, info.pose);
    },
  );
}

export async function updateMarkerInfo(
  id: string,
  patch: Partial<Pick<MapMarker, 'sizeM' | 'markerFrameId'>>,
): Promise<{ ok: boolean; error: string | null }> {
  const s = useMapStore.getState();
  const marker = s.markers[id];
  if (!marker) return { ok: true, error: null };

  if (patch.sizeM !== undefined) {
    const sizeError = validateSize(patch.sizeM);
    if (sizeError !== null) {
      s.setMutationError(sizeError);
      return { ok: false, error: sizeError };
    }
  }

  const merged = { ...marker, ...patch };
  return run(
    () => clover2Api.map.edit(marker.id, markerToInfo(merged, s.markerUi[id])),
    () => s.setMarkerInfoLocal(id, patch),
  );
}

export async function addMarker(): Promise<{ ok: boolean; error: string | null }> {
  const s = useMapStore.getState();
  const count = Object.keys(s.markers).length;
  const maxId = count ? Math.max(...Object.values(s.markers).map((m) => m.id)) : -1;
  const id = maxId + 1;
  const info: MarkerInfo = {
    id,
    type: 'fixed',
    size: DEFAULT_MARKER_SIZE_M,
    marker_frame_id: `Marker ${String(id).padStart(2, '0')}`,
    pose: {
      x: (count * 0.15) % 3,
      y: Math.floor(count / 5) * 0.15,
      z: 0.01,
      roll: 0,
      pitch: 0,
      yaw: 0,
    },
  };

  return run(
    () => clover2Api.map.add(info),
    () => {
      s.addMarkerLocal(info);
      s.seedUiFor({ [String(id)]: { id, type: 'fixed', sizeM: info.size, markerFrameId: info.marker_frame_id, pose: info.pose ?? null } });
      s.selectMarker(String(id));
    },
  );
}

export async function deleteMarkers(ids: string[]): Promise<{ ok: boolean; error: string | null }> {
  const s = useMapStore.getState();
  s.setMutationError(null);

  const results = await Promise.allSettled(ids.map((id) => clover2Api.map.delete(Number(id))));
  const failed = results.find(
    (r): r is PromiseRejectedResult | PromiseFulfilledResult<ModifyResult> =>
      r.status === 'rejected' || (r.status === 'fulfilled' && !r.value.success),
  );

  if (failed !== undefined) {
    const message =
      failed.status === 'rejected'
        ? failed.reason instanceof ApiError
          ? failed.reason.message
          : String(failed.reason)
        : failed.value.error_message || 'Unknown error';
    s.setMutationError(message);
    return { ok: false, error: message };
  }

  ids.forEach((id) => s.removeMarkerLocal(id));
  s.deselectAll();
  void s.reloadMap();
  return { ok: true, error: null };
}

export function downloadProject(): void {
  const data = exportProjectSnapshot();
  const name = useMapStore.getState().mapMeta?.name || DEFAULT_PROJECT_NAME;
  const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `${name.replace(/\s+/g, '-').toLowerCase()}-${Date.now()}.json`;
  a.click();
  URL.revokeObjectURL(url);
}

export function exportProjectSnapshot(): ProjectFile {
  const s = useMapStore.getState();
  const dictionary = s.mapMeta?.dictionary ?? 'DICT_4X4_1000';

  const markers: MarkerSnapshot[] = Object.values(s.markers).map((m) => {
    const ui = s.markerUi[String(m.id)];
    const positionExpr = ui?.positionExpr
      ?? (m.pose ? vec3ToPositionExpr([m.pose.x, m.pose.y, m.pose.z]) : [...DEFAULT_POSITION_EXPR]);
    const rotationExpr = ui?.rotationExpr
      ?? (m.pose ? eulerDegToRotationExpr(rpyToEulerYxzDeg(m.pose.roll, m.pose.pitch, m.pose.yaw)) : [...DEFAULT_ROTATION_EXPR]);

    return {
      id: String(m.id),
      markerId: m.id,
      dictionary,
      label: m.markerFrameId,
      sizeM: m.sizeM,
      positionExpr,
      rotationExpr,
      visible: ui?.visible ?? true,
    };
  });

  return {
    version: 1,
    meta: {
      name: s.mapMeta?.name || DEFAULT_PROJECT_NAME,
      description: '',
      createdAt: new Date().toISOString(),
      gridStepMm: DEFAULT_GRID_STEP_MM,
      angleSnapDeg: 15,
    },
    markers,
    dictionaryName: dictionary,
  };
}
