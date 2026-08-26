import { cn } from '../../lib/cn';
import type { RosLogEntry, RosLogLevelName } from '../../types/rosout.ts';

const LEVEL_CLASSES: Record<RosLogLevelName, string> = {
  debug: 'text-ink-faint',
  info: 'text-ink',
  warn: 'text-warning',
  error: 'text-error',
  fatal: 'text-error font-bold',
  unknown: 'text-ink-muted',
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
    <div className="flex items-baseline gap-3 px-3 py-1 border-b border-border-soft text-xs">
      <span className="text-ink-faint shrink-0 tabular-nums">{formatTime(entry)}</span>
      <span className={cn('w-14 shrink-0 font-semibold', LEVEL_CLASSES[entry.level])}>
        {levelLabel(entry)}
      </span>
      <span className="text-ink-muted shrink-0 max-w-48 truncate" title={entry.name}>
        {entry.name}
      </span>
      <span className="text-ink break-all flex-1">{entry.msg}</span>
      {source !== null && (
        <span className="text-ink-faint shrink-0" title={entry.file}>
          {source}
        </span>
      )}
    </div>
  );
}
