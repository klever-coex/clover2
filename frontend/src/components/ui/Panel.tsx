import { useId, useState } from 'react';
import type { ReactNode } from 'react';
import { ChevronDown } from 'lucide-react';

import { cn } from '../../lib/cn.ts';
import { Badge } from './Badge.tsx';

interface PanelProps {
  title?: ReactNode;
  count?: number;
  actions?: ReactNode;
  padded?: boolean;
  className?: string;
  collapsible?: boolean;
  defaultOpen?: boolean;
  children: ReactNode;
}

export function Panel({
  title,
  count,
  actions,
  padded = true,
  className,
  collapsible = false,
  defaultOpen = true,
  children,
}: PanelProps) {
  const [isOpen, setIsOpen] = useState(defaultOpen);
  const contentId = useId();

  return (
    <section
      className={cn('bg-surface-2 border border-line rounded-panel', className)}
    >
      {title !== undefined && (
        <header
          className={cn(
            'flex items-center gap-2 px-4 py-3',
            isOpen && 'border-b border-line',
          )}
        >
          {collapsible ? (
            <button
              type="button"
              onClick={() => setIsOpen((value) => !value)}
              aria-expanded={isOpen}
              aria-controls={contentId}
              className="flex flex-1 items-center gap-2 text-left rounded-row focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60"
            >
              <ChevronDown
                size={16}
                className={cn(
                  'shrink-0 text-ink-muted transition-transform duration-normal',
                  isOpen && 'rotate-180',
                )}
              />
              <span className="text-sm font-semibold text-ink">{title}</span>
            </button>
          ) : (
            <h2 className="flex-1 text-sm font-semibold text-ink">{title}</h2>
          )}
          {count !== undefined && <Badge tone="neutral">{count}</Badge>}
          {actions}
        </header>
      )}
      <div
        id={contentId}
        className={cn(
          'grid transition-[grid-template-rows] duration-normal',
          isOpen ? 'grid-rows-[1fr]' : 'grid-rows-[0fr]',
        )}
      >
        <div className={cn('overflow-hidden min-h-0', isOpen ? 'visible' : 'invisible')}>
          <div className={padded ? 'p-3' : undefined}>{children}</div>
        </div>
      </div>
    </section>
  );
}
