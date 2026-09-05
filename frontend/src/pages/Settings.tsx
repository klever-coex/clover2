import { useTranslation } from 'react-i18next';

import { SettingsSection } from '../components/settings/SettingsSection.tsx';
import { CapabilityGate } from '../components/ros2/CapabilityGate.tsx';
import { Button } from '@/components/ui/button';
import { ErrorState } from '../components/common/ErrorState.tsx';
import { LoadingState } from '../components/common/LoadingState.tsx';
import { usePageHeader } from '@/store/usePageHeader';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useRosCapability } from '@/hooks/useRosCapability';
import { useSettingsConfig } from '@/hooks/useSettingsConfig';
import { confirmDialog } from '@/store/useConfirmStore';

export function Settings() {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();
  const capability = useRosCapability('settings');
  const config = useSettingsConfig();

  const handleResetAll = async () => {
    const confirmed = await confirmDialog({
      message: t('settings.resetAllConfirm'),
      tone: 'danger',
      confirmLabel: t('settings.resetAll'),
    });
    if (confirmed) {
      config.resetAll();
    }
  };

  usePageHeader([], (
    <div className="flex items-center gap-2">
      {config.dirty && config.saveError !== null && (
        <span role="alert" className="text-xs text-destructive max-w-72">
          {config.saveError}
        </span>
      )}
      <Button variant="secondary" size="sm" onClick={() => void handleResetAll()}>
        {t('settings.resetAll')}
      </Button>
      <Button
        size="sm"
        disabled={!config.dirty || config.saving}
        onClick={() => void config.save()}
      >
        {config.saving ? t('settings.saving') : t('settings.save')}
      </Button>
    </div>
  ));

  return (
    <div className="p-6">
      <div>
        <CapabilityGate capability={capability} noCapability={t('settings.noCapability')}>
          {config.loading && config.root === null ? (
            <LoadingState />
          ) : config.error !== null ? (
            <ErrorState message={errorMessage(config.error)} onRetry={() => void config.reload()} />
          ) : config.root !== null ? (
            <div className="flex flex-col gap-4">
              {(config.root.children ?? []).map((section) => (
                <SettingsSection
                  key={section.name}
                  node={section}
                  path={[section.name]}
                  onValue={config.setValue}
                  onReset={config.resetField}
                />
              ))}
            </div>
          ) : null}
        </CapabilityGate>
      </div>
    </div>
  );
}
