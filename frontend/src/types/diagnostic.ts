import type { RosJsonValue } from './stream.ts';

export type DiagnosticLevelName = 'ok' | 'warn' | 'error' | 'stale';

export interface DiagnosticEntry {
  name: string;
  level: DiagnosticLevelName;
  levelValue: number;
  message: string;
  hardwareId: string;
  values: [string, string][];
  updatedAt: number;
}

export interface DiagnosticPoint {
  t: number;
  levelValue: number;
}

const LEVEL_NAMES: Record<number, DiagnosticLevelName> = {
  0: 'ok',
  1: 'warn',
  2: 'error',
  3: 'stale',
};

export { LEVEL_NAMES };

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

function parseEntry(value: RosJsonValue, now: number): DiagnosticEntry | null {
  const record = asRecord(value);
  if (record === null) return null;

  const name = asString(record.name);
  const message = asString(record.message);
  if (name === undefined || message === undefined) return null;

  const levelValue = asNumber(record.level) ?? 0;
  const hardwareId = asString(record.hardware_id) ?? '';

  const values: [string, string][] = [];
  if (Array.isArray(record.values)) {
    for (const item of record.values) {
      const kv = asRecord(item);
      if (kv === null) continue;
      const key = asString(kv.key);
      const val = asString(kv.value);
      if (key !== undefined && val !== undefined) values.push([key, val]);
    }
  }

  return {
    name,
    level: LEVEL_NAMES[levelValue] ?? 'stale',
    levelValue,
    message,
    hardwareId,
    values,
    updatedAt: now,
  };
}

export function parseDiagnosticArray(
  value: RosJsonValue,
  now: number = Date.now(),
): DiagnosticEntry[] {
  const record = asRecord(value);
  if (record === null || !Array.isArray(record.status)) return [];

  const entries: DiagnosticEntry[] = [];
  for (const item of record.status) {
    const entry = parseEntry(item, now);
    if (entry !== null) entries.push(entry);
  }
  return entries;
}

export function worstLevelValue(levels: Iterable<number>): number {
  let worst = -1;
  for (const level of levels) {
    if (level > worst) worst = level;
  }
  return worst;
}

export function effectiveLevelValue(entry: DiagnosticEntry, now: number, staleTimeoutMs: number): number {
  const seen = now - entry.updatedAt;
  return seen > staleTimeoutMs ? 3 : entry.levelValue;
}
