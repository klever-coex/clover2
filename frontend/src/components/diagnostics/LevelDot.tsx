import { cn } from '@/lib/utils';
import { LEVEL_BG_CLASS } from './levels.ts';
import type { DiagnosticLevelName } from '@/types/diagnostic.ts';

interface LevelDotProps {
  level: DiagnosticLevelName;
  className?: string;
}

export function LevelDot({ level, className }: LevelDotProps) {
  return (
    <span
      aria-hidden
      className={cn('size-2 shrink-0 rounded-full', LEVEL_BG_CLASS[level], className)}
    />
  );
}
