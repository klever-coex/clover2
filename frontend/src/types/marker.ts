import type { MarkerPose } from './map.ts';

export type Vec3 = readonly [number, number, number];
export type Quat4 = readonly [number, number, number, number];

export type ArUcoDictionary =
  | 'DICT_4X4_1000'
  | 'DICT_5X5_1000'
  | 'DICT_6X6_1000'
  | 'DICT_7X7_1000'
  | 'DICT_ARUCO_ORIGINAL'
  | 'DICT_APRILTAG_16h5'
  | 'DICT_APRILTAG_25h9'
  | 'DICT_APRILTAG_36h10'
  | 'DICT_APRILTAG_36h11';

export interface DictionaryMeta {
  name: ArUcoDictionary;
  gridSize: number;
  maxMarkerId: number;
  totalCells: number;
}

export interface DictionaryData {
  meta: DictionaryMeta;
  bytesList: Uint8Array[];
}

export type MarkerType = 'fixed' | 'static' | 'dynamic';

/** Backend truth (one per GET /api/map marker). Store key is String(id). */
export interface MapMarker {
  /** Backend marker id — also the expression variable `id`. */
  id: number;
  type: MarkerType;
  sizeM: number;
  markerFrameId: string;
  /** Present only for fixed markers. */
  pose: MarkerPose | null;
}

/** Session-only editing layer; seeded from pose on first sight, never refetched. */
export interface MarkerUi {
  /** mm, constants or expressions f(id). */
  positionExpr: [string, string, string];
  /** degrees, Euler YXZ convention. */
  rotationExpr: [string, string, string];
  visible: boolean;
}

export interface ProjectMeta {
  name: string;
  description: string;
  createdAt: string;
  gridStepMm: number;
  angleSnapDeg: number;
}

/** web3d ProjectFile v1 marker shape, kept for the Export snapshot only. */
export interface MarkerSnapshot {
  id: string;
  markerId: number;
  dictionary: ArUcoDictionary;
  label: string;
  sizeM: number;
  positionExpr: [string, string, string];
  rotationExpr: [string, string, string];
  visible: boolean;
}

export interface ProjectFile {
  version: 1;
  meta: ProjectMeta;
  markers: MarkerSnapshot[];
  dictionaryName: ArUcoDictionary;
}
