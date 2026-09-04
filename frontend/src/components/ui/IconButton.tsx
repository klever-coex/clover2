import type { ButtonHTMLAttributes } from 'react';
import type { LucideIcon } from 'lucide-react';

import { cn } from '../../lib/cn.ts';
import { buttonVariants } from '../../lib/uiStyles.ts';
import { buttonBase } from './Button.tsx';

interface IconButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  icon: LucideIcon;
  label: string;
  size?: 'sm' | 'md';
  variant?: 'ghost' | 'secondary' | 'primary';
  expanded?: boolean;
}

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
        buttonVariants[variant],
        size === 'sm' ? 'p-1.5' : 'p-2',
        className,
      )}
      {...props}
    >
      <Icon size={20} />
    </button>
  );
}
