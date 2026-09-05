import { useId, useState } from 'react';
import type { ReactNode } from 'react';
import { ChevronRight } from 'lucide-react';

import { cn } from '@/lib/utils';

interface CollapsibleProps {
  label: ReactNode;
  children: ReactNode;
  defaultOpen?: boolean;
  open?: boolean;
  onToggle?: () => void;
}

export function JsonCollapsible({ label, children, defaultOpen = true, open, onToggle }: CollapsibleProps) {
  const [internalOpen, setInternalOpen] = useState(defaultOpen);
  const isOpen = open ?? internalOpen;
  const contentId = useId();

  const toggle = () => {
    onToggle?.();
    setInternalOpen(!internalOpen);
  };

  return (
    <div className="ml-4">
      <button
        type="button"
        onClick={toggle}
        aria-expanded={isOpen}
        aria-controls={contentId}
        className="flex items-center gap-1 rounded-row font-semibold text-json-key transition-colors duration-fast hover:text-foreground focus-visible:ring-2 focus-visible:ring-ring/60 focus-visible:outline-none"
      >
        <ChevronRight
          size={14}
          className={cn('shrink-0 transition-transform duration-normal', isOpen && 'rotate-90')}
        />
        <span className="mr-1">{label}:</span>
      </button>
      {/* Children may be rendered lazily by the caller; the node stays live for aria-controls. */}
      <div id={contentId} className="ml-4" hidden={!isOpen}>
        {children}
      </div>
    </div>
  );
}
