import type { ArUcoDictionary } from '../types/marker.ts';

export const DEFAULT_DICTIONARY: ArUcoDictionary = 'DICT_4X4_1000';
export const DEFAULT_MARKER_SIZE_M = 0.33; // large enough to see ArUco pattern
export const DEFAULT_POSITION_EXPR: [string, string, string] = ['0', '0', '0']; // mm: X=right, Y=forward, Z=up (height)
export const DEFAULT_ROTATION_EXPR: [string, string, string] = ['0', '0', '0']; // deg: flat on ground
export const DEFAULT_GRID_STEP_MM = 50; // 5cm grid snap
export const DEFAULT_PROJECT_NAME = 'Untitled ArUco Map';
