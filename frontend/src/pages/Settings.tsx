import { useTranslation } from 'react-i18next';

import { SettingsSection } from '../components/settings/SettingsSection.tsx';
import { CapabilityGate } from '../components/ros2/CapabilityGate.tsx';
import { Button } from '../components/ui/Button.tsx';
import { ErrorState } from '../components/ui/ErrorState.tsx';
import { LoadingState } from '../components/ui/LoadingState.tsx';
import { PageHeader } from '../components/ui/PageHeader.tsx';
import { useApiErrorMessage } from '../hooks/useApiErrorMessage.ts';
import { useRosCapability } from '../hooks/useRosCapability.ts';
import { useSettingsConfig } from '../hooks/useSettingsConfig.ts';
import { confirmDialog } from '../store/useConfirmStore.ts';

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

  return (
    <div className="p-6">
      <PageHeader
        title={t('settings.title')}
        actions={
          <div className="flex items-center gap-2">
            {config.dirty && config.saveError !== null && (
              <span role="alert" className="text-xs text-error max-w-72">
                {config.saveError}
              </span>
            )}
            <Button variant="secondary" size="sm" onClick={() => void handleResetAll()}>
              {t('settings.resetAll')}
            </Button>
            <Button
              variant="primary"
              size="sm"
              disabled={!config.dirty || config.saving}
              onClick={() => void config.save()}
            >
              {config.saving ? t('settings.saving') : t('settings.save')}
            </Button>
          </div>
        }
      />

      <div className="mt-4">
        <CapabilityGate capability={capability} noCapability={t('settings.noCapability')}>
          {config.loading && config.root === null ? (
            <LoadingState />
          ) : config.error !== null ? (
            <ErrorState message={errorMessage(config.error)} onRetry={() => void config.reload()} />
          ) : config.root !== null ? (
            <div className="space-y-4">
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
