import { useTranslation } from 'react-i18next';

import { Button } from './Button.tsx';

interface ErrorStateProps {
  message: string;
  onRetry?: () => void;
}

export function ErrorState({ message, onRetry }: ErrorStateProps) {
  const { t } = useTranslation();

  return (
    <div className="flex flex-col items-center gap-3 p-8">
      <p className="text-error text-center">{message}</p>
      {onRetry && (
        <Button variant="primary" size="sm" onClick={onRetry}>
          {t('common.retry')}
        </Button>
      )}
    </div>
  );
}
