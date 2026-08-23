import type { SelectHTMLAttributes } from 'react';

import { cn } from '../../lib/cn.ts';

type SelectProps = SelectHTMLAttributes<HTMLSelectElement>;

/**
 * Native select styled to match SearchInput. Consumers must provide an
 * accessible name: either aria-label or an associated <label htmlFor>.
 */
export function Select({ className, ...props }: SelectProps) {
  return (
    <select
      className={cn(
        'rounded-row border border-line bg-surface-1 px-3 py-2 text-sm text-ink outline-none transition-colors duration-fast focus:border-accent/60 focus:ring-2 focus:ring-accent/20 disabled:opacity-50',
        className,
      )}
      {...props}
    />
  );
}
