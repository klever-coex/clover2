import { useTranslation } from 'react-i18next';

import { cn } from '../../lib/cn.ts';
import { Spinner } from './Spinner.tsx';

interface LoadingStateProps {
  /** inline: page section block; centered: full-height page placeholder. */
  variant?: 'inline' | 'centered';
  size?: number;
  /** Passthrough to the Spinner (e.g. text-error tint). */
  className?: string;
}

export function LoadingState({
  variant = 'inline',
  size = 32,
  className,
}: LoadingStateProps) {
  const { t } = useTranslation();

  return (
    <div
      role="status"
      className={cn(
        variant === 'inline'
          ? 'flex justify-center py-8'
          : 'flex justify-center items-center min-h-[60vh]',
      )}
    >
      <Spinner size={size} className={className} />
      <span className="sr-only">{t('common.loading')}</span>
    </div>
  );
}
