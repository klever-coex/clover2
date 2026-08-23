import { Loader2 } from 'lucide-react';

import { cn } from '../../lib/cn.ts';

interface SpinnerProps {
  size?: number;
  className?: string;
}

export function Spinner({ size = 28, className }: SpinnerProps) {
  return (
    <Loader2
      className={cn('animate-spin motion-reduce:animate-none text-accent', className)}
      size={size}
    />
  );
}
