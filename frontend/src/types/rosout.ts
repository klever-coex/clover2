import type { RosJsonValue } from './stream.ts';

export interface RosLogStamp {
  sec: number;
  nanosec: number;
}

export type RosLogLevelName = 'debug' | 'info' | 'warn' | 'error' | 'fatal' | 'unknown';

export interface RosLogEntry {
  seq: number;
  receivedAt: number;
  level: RosLogLevelName;
  levelValue: number;
  name: string;
  msg: string;
  file?: string;
  line?: number;
  stamp?: RosLogStamp;
}

const LEVEL_NAMES: Record<number, RosLogLevelName> = {
  10: 'debug',
  20: 'info',
  30: 'warn',
  40: 'error',
  50: 'fatal',
};

function asRecord(value: RosJsonValue | undefined): Record<string, RosJsonValue> | null {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) return null;
  return value;
}

function asString(value: RosJsonValue | undefined): string | undefined {
  return typeof value === 'string' ? value : undefined;
}

function asNumber(value: RosJsonValue | undefined): number | undefined {
  return typeof value === 'number' && Number.isFinite(value) ? value : undefined;
}

export function parseRosLogEntry(value: RosJsonValue, seq: number): RosLogEntry | null {
  const record = asRecord(value);
  if (record === null) return null;

  const level = asNumber(record.level);
  const name = asString(record.name);
  const msg = asString(record.msg);
  if (level === undefined || name === undefined || msg === undefined) return null;

  const stampRecord = asRecord(record.stamp);
  const sec = stampRecord !== null ? asNumber(stampRecord.sec) : undefined;
  const nanosec = stampRecord !== null ? asNumber(stampRecord.nanosec) : undefined;

  return {
    seq,
    receivedAt: Date.now(),
    level: LEVEL_NAMES[level] ?? 'unknown',
    levelValue: level,
    name,
    msg,
    file: asString(record.file),
    line: asNumber(record.line),
    stamp: sec !== undefined && nanosec !== undefined ? { sec, nanosec } : undefined,
  };
}
