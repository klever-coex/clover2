import { useTranslation } from 'react-i18next';

import { cn } from '../../lib/cn.ts';
import { centerFill } from '../../lib/uiStyles.ts';
import { Spinner } from './Spinner.tsx';

interface LoadingStateProps {
  variant?: 'inline' | 'centered';
  size?: number;
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
          : cn('flex justify-center items-center', centerFill),
      )}
    >
      <Spinner size={size} className={className} />
      <span className="sr-only">{t('common.loading')}</span>
    </div>
  );
}
