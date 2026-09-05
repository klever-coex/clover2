import { useTranslation } from 'react-i18next';

import { Alert, AlertDescription } from '@/components/ui/alert.tsx';
import { Button } from '@/components/ui/button.tsx';

interface ErrorStateProps {
  message: string;
  onRetry?: () => void;
}

export function ErrorState({ message, onRetry }: ErrorStateProps) {
  const { t } = useTranslation();

  return (
    <div className="flex flex-col items-center gap-3 p-8">
      <Alert
        variant="destructive"
        role="alert"
        className="w-auto max-w-sm border-none bg-transparent px-0 py-0 text-center shadow-none"
      >
        <AlertDescription className="text-destructive">{message}</AlertDescription>
      </Alert>
      {onRetry && (
        <Button size="sm" onClick={onRetry}>
          {t('common.retry')}
        </Button>
      )}
    </div>
  );
}
