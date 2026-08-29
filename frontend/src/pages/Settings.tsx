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

/** Clover2 configuration editor backed by the settings_server plugin. */
export function Settings() {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();
  const capability = useRosCapability('settings');
  const config = useSettingsConfig();

  const handleResetAll = () => {
    if (confirm(t('settings.resetAllConfirm'))) {
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
              <span className="text-xs text-error max-w-72 truncate" title={config.saveError}>
                {config.saveError}
              </span>
            )}
            <Button variant="secondary" size="sm" onClick={handleResetAll}>
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
