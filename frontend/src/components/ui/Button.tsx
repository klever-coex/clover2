import type { ButtonHTMLAttributes, Ref } from 'react';

import { cn } from '../../lib/cn.ts';
import { buttonVariants } from '../../lib/uiStyles.ts';

export const buttonBase =
  'inline-flex items-center justify-center gap-2 font-medium rounded-row transition-colors duration-fast focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60 disabled:opacity-50 disabled:pointer-events-none';

interface ButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: keyof typeof buttonVariants;
  size?: 'sm' | 'md';
  ref?: Ref<HTMLButtonElement>;
}

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
      className={cn(buttonBase, buttonVariants[variant], sizeClasses[size], className)}
      {...props}
    />
  );
}
