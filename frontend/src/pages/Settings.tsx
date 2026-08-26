import { useTranslation } from 'react-i18next';

import { PageHeader } from '../components/ui/PageHeader.tsx';

export function Settings() {
  const { t } = useTranslation();

  return (
    <div className="p-6">
      <PageHeader title={t('sidebar.settings')} />
      <p className="mt-4 text-sm text-ink-faint">{t('common.underConstruction')}</p>
    </div>
  );
}
