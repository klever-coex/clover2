import { useCallback, useMemo } from 'react';
import { useTranslation } from 'react-i18next';

import { SettingsSection } from '../components/settings/SettingsSection.tsx';
import { CapabilityGate } from '../components/ros2/CapabilityGate.tsx';
import { InfoLink } from '../components/common/InfoLink.tsx';
import { Button } from '@/components/ui/button';
import { ErrorState } from '../components/common/ErrorState.tsx';
import { LoadingState } from '../components/common/LoadingState.tsx';
import { usePageHeader } from '@/store/usePageHeader';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useSettingsConfig } from '@/hooks/useSettingsConfig';
import { confirmDialog } from '@/store/useConfirmStore';

export function Settings() {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();
  const config = useSettingsConfig();
  const { dirty, saving, saveError, save, resetAll, setValue, resetField, reload } = config;

  const handleResetAll = useCallback(async () => {
    const confirmed = await confirmDialog({
      message: t('settings.resetAllConfirm'),
      tone: 'danger',
      confirmLabel: t('settings.resetAll'),
    });
    if (confirmed) {
      resetAll();
    }
  }, [t, resetAll]);

  const headerActions = useMemo(
    () => (
      <div className="flex items-center gap-2">
        {dirty && saveError !== null && (
          <span role="alert" className="text-xs text-destructive max-w-72">
            {saveError}
          </span>
        )}
        <InfoLink docKey="programming.settings" />
        <Button variant="secondary" size="sm" onClick={() => void handleResetAll()}>
          {t('settings.resetAll')}
        </Button>
        <Button size="sm" disabled={!dirty || saving} onClick={() => void save()}>
          {saving ? t('settings.saving') : t('settings.save')}
        </Button>
      </div>
    ),
    [t, dirty, saving, saveError, save, handleResetAll],
  );

  usePageHeader([], headerActions);

  return (
    <div className="p-6">
      <div>
        <CapabilityGate name="settings" noCapability={t('settings.noCapability')}>
          {config.loading && config.root === null ? (
            <LoadingState />
          ) : config.error !== null ? (
            <ErrorState message={errorMessage(config.error)} onRetry={() => void reload()} />
          ) : config.root !== null ? (
            <div className="flex flex-col gap-4">
              {(config.root.children ?? []).map((section) => (
                <SettingsSection
                  key={section.name}
                  node={section}
                  path={[section.name]}
                  onValue={setValue}
                  onReset={resetField}
                />
              ))}
            </div>
          ) : null}
        </CapabilityGate>
      </div>
    </div>
  );
}
