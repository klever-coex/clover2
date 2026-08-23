import type { InputHTMLAttributes } from 'react';

import { cn } from '../../lib/cn.ts';

type SearchInputProps = InputHTMLAttributes<HTMLInputElement>;

export function SearchInput({ className, ...props }: SearchInputProps) {
  return (
    <input
      type="text"
      className={cn(
        'w-full rounded-row border border-line bg-surface-1 px-3 py-2 text-sm text-ink placeholder:text-ink-faint outline-none transition-colors duration-fast focus:border-accent/60 focus:ring-2 focus:ring-accent/20',
        className,
      )}
      {...props}
    />
  );
}
