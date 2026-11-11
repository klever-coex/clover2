import type { DiagnosticLevelName } from '@/types/diagnostic.ts';

/** Dot / fill background classes per level. */
export const LEVEL_BG_CLASS: Record<DiagnosticLevelName, string> = {
  ok: 'bg-success',
  warn: 'bg-warning',
  error: 'bg-destructive',
  stale: 'bg-muted-foreground/50',
};

/** Text color classes per level. */
export const LEVEL_TEXT_CLASS: Record<DiagnosticLevelName, string> = {
  ok: 'text-success',
  warn: 'text-warning',
  error: 'text-destructive',
  stale: 'text-muted-foreground',
};

/** Sparkline bucket fill per numeric level (-1 = no data). */
const LEVEL_BUCKET_CLASS: Record<number, string> = {
  [-1]: 'bg-muted/50',
  0: 'bg-success',
  1: 'bg-warning',
  2: 'bg-destructive',
  3: 'bg-muted-foreground/60',
};

const NO_DATA_CLASS = 'bg-muted/50';

export function bucketClass(levelValue: number): string {
  if (levelValue < 0) return NO_DATA_CLASS;
  return LEVEL_BUCKET_CLASS[Math.min(levelValue, 3)] ?? NO_DATA_CLASS;
}
