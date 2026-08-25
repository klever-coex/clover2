import type { ReactNode } from 'react';

import { cn } from '../../lib/cn.ts';

export type BadgeTone = 'neutral' | 'accent' | 'success' | 'warning' | 'error';

interface BadgeProps {
  tone?: BadgeTone;
  className?: string;
  children: ReactNode;
}

const toneClasses: Record<BadgeTone, string> = {
  neutral: 'bg-surface-3 text-ink-muted',
  accent: 'bg-accent-soft text-accent-text',
  success: 'bg-success/12 text-success',
  warning: 'bg-warning/12 text-warning',
  error: 'bg-error/12 text-error',
};

export function Badge({ tone = 'neutral', className, children }: BadgeProps) {
  return (
    <span
      className={cn(
        'inline-flex items-center gap-1 rounded-full px-3 py-1 text-xs font-medium whitespace-nowrap',
        toneClasses[tone],
        className,
      )}
    >
      {children}
    </span>
  );
}
