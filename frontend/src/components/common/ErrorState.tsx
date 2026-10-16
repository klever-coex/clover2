import { useTranslation } from 'react-i18next';

import { Button } from '@/components/ui/button.tsx';

interface ErrorStateProps {
  message: string;
  onRetry?: () => void;
}

export function ErrorState({ message, onRetry }: ErrorStateProps) {
  const { t } = useTranslation();

  return (
    <div className="flex flex-col items-center gap-3 p-8">
      <p role="alert" className="max-w-sm text-center text-sm text-destructive">
        {message}
      </p>
      {onRetry && (
        <Button size="sm" onClick={onRetry}>
          {t('common.retry')}
        </Button>
      )}
    </div>
  );
}
