import { useTranslation } from 'react-i18next';

import type { ApiError } from '../types/errors.ts';

export function useApiErrorMessage(): (error: ApiError) => string {
  const { t } = useTranslation();

  return (error) => {
    if (error.kind === 'network') return t('errors.backendUnavailable');
    if (error.kind === 'timeout') return t('errors.timeout');
    return error.message;
  };
}
