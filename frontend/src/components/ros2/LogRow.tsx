import { cn } from '@/lib/utils';
import type { RosLogEntry, RosLogLevelName } from '@/types/rosout';

const LEVEL_CLASSES: Record<RosLogLevelName, string> = {
  debug: 'text-muted-foreground/80',
  info: 'text-foreground',
  warn: 'text-warning',
  error: 'text-destructive',
  fatal: 'text-destructive font-bold',
  unknown: 'text-muted-foreground',
};

const LEVEL_LABELS: Record<Exclude<RosLogLevelName, 'unknown'>, string> = {
  debug: 'DEBUG',
  info: 'INFO',
  warn: 'WARN',
  error: 'ERROR',
  fatal: 'FATAL',
};

function levelLabel(entry: RosLogEntry): string {
  return entry.level === 'unknown' ? `L${entry.levelValue}` : LEVEL_LABELS[entry.level];
}

function formatTime(entry: RosLogEntry): string {
  const ms =
    entry.stamp !== undefined
      ? entry.stamp.sec * 1000 + entry.stamp.nanosec / 1_000_000
      : entry.receivedAt;
  const date = new Date(ms);
  const pad = (value: number, width = 2) => String(value).padStart(width, '0');
  return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}.${pad(date.getMilliseconds(), 3)}`;
}

export function LogRow({ entry }: { entry: RosLogEntry }) {
  const source =
    entry.file !== undefined
      ? `${entry.file.split('/').pop()}${entry.line !== undefined ? `:${entry.line}` : ''}`
      : null;

  return (
    // content-visibility skips layout/paint for rows scrolled out of view —
    // a cheap native virtualization for the unbounded log list.
    <div className="flex items-baseline gap-3 px-3 py-1 border-b border-border text-xs [content-visibility:auto] [contain-intrinsic-size:auto_24px]">
      <span className="text-muted-foreground/80 shrink-0 tabular-nums">{formatTime(entry)}</span>
      <span className={cn('w-14 shrink-0 font-semibold', LEVEL_CLASSES[entry.level])}>
        {levelLabel(entry)}
      </span>
      <span className="text-muted-foreground shrink-0 max-w-48 truncate" title={entry.name}>
        {entry.name}
      </span>
      <span className="text-foreground break-all flex-1">{entry.msg}</span>
      {source !== null && (
        <span className="text-muted-foreground/80 shrink-0" title={entry.file}>
          {source}
        </span>
      )}
    </div>
  );
}
