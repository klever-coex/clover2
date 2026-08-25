import { useId, useState } from 'react';
import type { ReactNode } from 'react';
import { ChevronRight } from 'lucide-react';

import { cn } from '../../lib/cn.ts';

interface CollapsibleProps {
  label: ReactNode;
  children: ReactNode;
  defaultOpen?: boolean;
}

/** Keyboard-accessible collapsible section (native button semantics). */
export function Collapsible({ label, children, defaultOpen = true }: CollapsibleProps) {
  const [isOpen, setIsOpen] = useState(defaultOpen);
  const contentId = useId();

  return (
    <div className="ml-4">
      <button
        type="button"
        onClick={() => setIsOpen(!isOpen)}
        aria-expanded={isOpen}
        aria-controls={contentId}
        className="flex items-center gap-1 font-semibold text-json-key hover:text-ink rounded transition-colors duration-fast focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60"
      >
        <ChevronRight
          size={14}
          className={cn('shrink-0 transition-transform duration-normal', isOpen && 'rotate-90')}
        />
        <span className="mr-1">{label}:</span>
      </button>
      {/* Children stay mounted so aria-controls always references a live node. */}
      <div id={contentId} className="ml-4" hidden={!isOpen}>
        {children}
      </div>
    </div>
  );
}
