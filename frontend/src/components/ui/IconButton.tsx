import type { ButtonHTMLAttributes } from 'react';
import type { LucideIcon } from 'lucide-react';

import { cn } from '../../lib/cn.ts';
import { buttonBase } from './Button.tsx';

interface IconButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  icon: LucideIcon;
  label: string;
  size?: 'sm' | 'md';
  variant?: 'ghost' | 'secondary' | 'primary';
  expanded?: boolean;
}

const variantClasses: Record<NonNullable<IconButtonProps['variant']>, string> = {
  ghost: 'text-ink-muted hover:text-ink hover:bg-surface-2',
  secondary: 'bg-surface-2 text-ink border border-line hover:bg-surface-3',
  primary:
    'bg-accent text-white hover:bg-accent-hover active:bg-accent-active',
};

export function IconButton({
  icon: Icon,
  label,
  size = 'md',
  variant = 'ghost',
  expanded,
  className,
  ...props
}: IconButtonProps) {
  return (
    <button
      aria-label={label}
      aria-expanded={expanded}
      className={cn(
        buttonBase,
        variantClasses[variant],
        size === 'sm' ? 'p-1.5' : 'p-2',
        className,
      )}
      {...props}
    >
      <Icon size={20} />
    </button>
  );
}
