export const MM_TO_M = 0.001;
export const M_TO_MM = 1000;
export const DEG_TO_RAD = Math.PI / 180;
export const RAD_TO_DEG = 180 / Math.PI;

export function mmToM(mm: number): number {
  return mm * MM_TO_M;
}

export function mToMm(m: number): number {
  return Math.round(m * M_TO_MM * 100) / 100; // round to 0.01mm
}

export function degToRad(deg: number): number {
  return deg * DEG_TO_RAD;
}

export function radToDeg(rad: number): number {
  let deg = rad * RAD_TO_DEG;
  deg = ((deg + 180) % 360 + 360) % 360 - 180;
  return Math.round(deg * 100) / 100; // round to 0.01°
}
