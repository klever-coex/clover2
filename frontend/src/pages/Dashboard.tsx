import { useTranslation } from 'react-i18next';


export function Dashboard() {
  const { t } = useTranslation();

  return (
    <div className="p-6">
      <p className="text-sm text-muted-foreground/80">{t('common.underConstruction')}</p>
    </div>
  );
}
