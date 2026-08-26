import type { ButtonHTMLAttributes } from 'react';

import { cn } from '../../lib/cn.ts';

export const buttonBase =
  'inline-flex items-center justify-center gap-2 font-medium rounded-row transition-colors duration-fast focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60 disabled:opacity-50 disabled:pointer-events-none';

interface ButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: 'primary' | 'secondary' | 'ghost' | 'danger';
  size?: 'sm' | 'md';
}

const variantClasses: Record<NonNullable<ButtonProps['variant']>, string> = {
  primary:
    'bg-accent text-white hover:bg-accent-hover active:bg-accent-active',
  secondary: 'bg-surface-2 text-ink border border-line hover:bg-surface-3',
  ghost: 'text-ink-muted hover:text-ink hover:bg-surface-2',
  danger: 'bg-error/15 text-error hover:bg-error/25',
};

const sizeClasses: Record<NonNullable<ButtonProps['size']>, string> = {
  sm: 'px-2.5 py-1.5 text-xs',
  md: 'px-3.5 py-2 text-sm',
};

export function Button({
  variant = 'primary',
  size = 'md',
  className,
  ...props
}: ButtonProps) {
  return (
    <button
      className={cn(buttonBase, variantClasses[variant], sizeClasses[size], className)}
      {...props}
    />
  );
}
