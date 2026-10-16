import { useTranslation } from 'react-i18next';

import { cn } from '@/lib/utils';
import { Spinner } from '@/components/ui/spinner.tsx';

interface LoadingStateProps {
  variant?: 'inline' | 'centered';
  className?: string;
}

export function LoadingState({ variant = 'inline', className }: LoadingStateProps) {
  const { t } = useTranslation();

  return (
    <div
      role="status"
      className={cn(
        'flex justify-center',
        variant === 'inline' ? 'py-8' : 'min-h-[60vh] items-center',
      )}
    >
      <Spinner className={cn('size-8 text-primary', className)} />
      <span className="sr-only">{t('common.loading')}</span>
    </div>
  );
}
