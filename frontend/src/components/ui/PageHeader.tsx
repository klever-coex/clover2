import type { ReactNode } from 'react';
import { Link } from 'react-router';
import { ChevronLeft } from 'lucide-react';

import { cn } from '../../lib/cn.ts';

interface PageHeaderProps {
  title: ReactNode;
  backTo?: { to: string; label: string };
  actions?: ReactNode;
  className?: string;
}

export function PageHeader({ title, backTo, actions, className }: PageHeaderProps) {
  return (
    <div className={cn('flex items-center justify-between gap-4 flex-wrap', className)}>
      <div>
        {backTo && (
          <Link
            to={backTo.to}
            className="inline-flex items-center gap-1 text-sm text-ink-muted hover:text-accent-text transition-colors duration-fast font-medium"
          >
            <ChevronLeft size={16} />
            {backTo.label}
          </Link>
        )}
        <h1 className="text-2xl font-bold text-ink break-all">{title}</h1>
      </div>
      {actions}
    </div>
  );
}
