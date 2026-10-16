import type { MouseEvent, ReactNode } from 'react';
import { cn } from '@/lib/utils';

interface CardRowProps {
  onSelect?: (event: MouseEvent<HTMLButtonElement>) => void;
  children: ReactNode;
  action?: ReactNode;
  variant?: 'card' | 'compact';
  active?: boolean;
  ariaPressed?: boolean;
  contentClassName?: string;
  className?: string;
  title?: string;
}

export function CardRow({
  onSelect,
  children,
  action,
  variant = 'card',
  active = false,
  ariaPressed,
  contentClassName,
  className,
  title,
}: CardRowProps) {
  const compact = variant === 'compact';

  return (
    <div
      className={cn(
        'flex items-stretch rounded-row transition-colors duration-fast',
        compact ? 'gap-0.5' : 'gap-1 border border-border bg-card hover:bg-muted',
        active && 'bg-secondary',
        className,
      )}
    >
      <button
        type="button"
        onClick={onSelect}
        title={title}
        aria-pressed={compact && ariaPressed !== undefined ? ariaPressed : undefined}
        className={cn(
          'min-w-0 flex-1 flex items-center justify-between gap-2 rounded-row text-left transition-colors duration-fast cursor-pointer focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring/60',
          compact
            ? active
              ? 'bg-secondary px-2 py-1 text-xs text-foreground'
              : 'px-2 py-1 text-xs text-muted-foreground hover:bg-muted hover:text-foreground'
            : 'p-3 text-sm text-foreground',
          contentClassName,
        )}
      >
        {children}
      </button>
      {action !== undefined && (
        <div className={cn('flex items-center', compact ? 'pr-0.5' : 'pr-1')}>{action}</div>
      )}
    </div>
  );
}
