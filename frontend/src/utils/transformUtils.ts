import * as THREE from 'three';
import type { Quat4, Vec3 } from '../types/marker.ts';
import type { MarkerPose } from '../types/map.ts';
import { radToDeg, degToRad, mmToM, mToMm } from '../constants/units.ts';
import { evaluateExpr } from './exprEval.ts';

const _euler = new THREE.Euler();
const _quat = new THREE.Quaternion();

export function quatToEulerDeg(q: Quat4): Vec3 {
  _quat.set(q[0], q[1], q[2], q[3]);
  _euler.setFromQuaternion(_quat, 'YXZ');
  return [
    radToDeg(_euler.x),
    radToDeg(_euler.y),
    radToDeg(_euler.z),
  ];
}

export function eulerDegToQuat(x: number, y: number, z: number): Quat4 {
  _euler.set(degToRad(x), degToRad(y), degToRad(z), 'YXZ');
  _quat.setFromEuler(_euler);
  return [_quat.x, _quat.y, _quat.z, _quat.w];
}

function safeEval(expr: string, markerId: number, fallback: number): number {
  try {
    const val = evaluateExpr(expr || '0', { id: markerId });
    return Number.isFinite(val) ? val : fallback;
  } catch {
    return fallback;
  }
}

export function resolvePosition(expr: [string, string, string], markerId: number): Vec3 {
  return [
    mmToM(safeEval(expr[0], markerId, 0)),
    mmToM(safeEval(expr[1], markerId, 0)),
    mmToM(safeEval(expr[2], markerId, 0)),
  ];
}

export function resolveRotation(expr: [string, string, string], markerId: number): Quat4 {
  const x = safeEval(expr[0], markerId, 0);
  const y = safeEval(expr[1], markerId, 0);
  const z = safeEval(expr[2], markerId, 0);
  return eulerDegToQuat(x, y, z);
}

function resolveRotationDeg(expr: [string, string, string], markerId: number): Vec3 {
  return [safeEval(expr[0], markerId, 0), safeEval(expr[1], markerId, 0), safeEval(expr[2], markerId, 0)];
}

function eulerYxzDegToRpy(xDeg: number, yDeg: number, zDeg: number): [number, number, number] {
  const q = eulerDegToQuat(xDeg, yDeg, zDeg);
  _quat.set(q[0], q[1], q[2], q[3]);
  _euler.setFromQuaternion(_quat, 'ZYX');
  return [_euler.x, _euler.y, _euler.z]; // [roll, pitch, yaw] rad
}

export function rpyToEulerYxzDeg(roll: number, pitch: number, yaw: number): Vec3 {
  _euler.set(roll, pitch, yaw, 'ZYX');
  _quat.setFromEuler(_euler);
  _euler.setFromQuaternion(_quat, 'YXZ');
  return [radToDeg(_euler.x), radToDeg(_euler.y), radToDeg(_euler.z)];
}

export function resolvePose(
  positionExpr: [string, string, string],
  rotationExpr: [string, string, string],
  markerId: number,
): MarkerPose {
  const [x, y, z] = resolvePosition(positionExpr, markerId);
  const [rd, pd, yd] = resolveRotationDeg(rotationExpr, markerId);
  const [roll, pitch, yaw] = eulerYxzDegToRpy(rd, pd, yd);
  return { x, y, z, roll, pitch, yaw };
}

export function vec3ToPositionExpr(v: Vec3): [string, string, string] {
  return [mToMm(v[0]).toString(), mToMm(v[1]).toString(), mToMm(v[2]).toString()];
}

export function eulerDegToRotationExpr(v: Vec3): [string, string, string] {
  return [v[0].toString(), v[1].toString(), v[2].toString()];
}

export function sanitizeVec3(v: Vec3): Vec3 {
  return [
    Number.isFinite(v[0]) ? v[0] : 0,
    Number.isFinite(v[1]) ? v[1] : 0,
    Number.isFinite(v[2]) ? v[2] : 0,
  ];
}
