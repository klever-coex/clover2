import { cn } from '@/lib/utils';
import { bucketClass } from './levels.ts';

interface LevelSparklineProps {
  buckets: number[];
  label: string;
  className?: string;
}

export function LevelSparkline({ buckets, label, className }: LevelSparklineProps) {
  return (
    <div
      role="img"
      aria-label={label}
      className={cn('flex h-3 w-full items-stretch gap-px', className)}
    >
      {buckets.map((levelValue, index) => (
        <span key={index} className={cn('flex-1 rounded-[2px]', bucketClass(levelValue))} />
      ))}
    </div>
  );
}
