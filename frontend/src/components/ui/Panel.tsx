import type { ReactNode } from 'react';

import { cn } from '../../lib/cn.ts';
import { Badge } from './Badge.tsx';

interface PanelProps {
  title?: ReactNode;
  count?: number;
  actions?: ReactNode;
  padded?: boolean;
  className?: string;
  children: ReactNode;
}

export function Panel({
  title,
  count,
  actions,
  padded = true,
  className,
  children,
}: PanelProps) {
  return (
    <section
      className={cn('bg-surface-2 border border-line rounded-panel', className)}
    >
      {title !== undefined && (
        <header className="flex items-center justify-between gap-2 px-4 py-3 border-b border-line">
          <div className="flex items-center gap-2">
            <h2 className="text-sm font-semibold text-ink">{title}</h2>
            {count !== undefined && <Badge tone="neutral">{count}</Badge>}
          </div>
          {actions}
        </header>
      )}
      <div className={padded ? 'p-4' : undefined}>{children}</div>
    </section>
  );
}
