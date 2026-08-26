import { useTranslation } from 'react-i18next';

import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { ErrorBoundary } from '../../components/map/ErrorBoundary.tsx';
import { SidePanel } from '../../components/map/panel/SidePanel.tsx';
import { SceneCanvas } from '../../components/map/scene/SceneCanvas.tsx';
import { ErrorState } from '../../components/ui/ErrorState.tsx';
import { LoadingState } from '../../components/ui/LoadingState.tsx';
import { PageHeader } from '../../components/ui/PageHeader.tsx';
import { cn } from '../../lib/cn.ts';
import { useApiErrorMessage } from '../../hooks/useApiErrorMessage.ts';
import { useMapKeyboardShortcuts } from '../../hooks/useMapKeyboardShortcuts.ts';
import { useCapabilityFetch } from '../../hooks/useCapabilityFetch.ts';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useMapStore } from '../../store/useMapStore.ts';
import { useMapUIStore } from '../../store/useMapUIStore.ts';

export default function MapPage() {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();
  const capability = useRosCapability('map');
  const markers = useMapStore((s) => s.markers);
  const mapLoading = useMapStore((s) => s.mapLoading);
  const mapError = useMapStore((s) => s.mapError);
  const reloadMap = useMapStore((s) => s.reloadMap);
  const sidePanelOpen = useMapUIStore((s) => s.sidePanelOpen);

  useCapabilityFetch(capability, reloadMap);

  useMapKeyboardShortcuts();

  return (
    <div className="p-6 h-full flex flex-col">
      <PageHeader title={t('map.title')} />

      <div className="mt-4 flex-1 min-h-0">
        <CapabilityGate capability={capability} noCapability={t('map.noCapability')}>
          {mapLoading && Object.keys(markers).length === 0 ? (
            <LoadingState variant="centered" />
          ) : mapError !== null ? (
            <ErrorState
              message={errorMessage(mapError)}
              onRetry={() => void reloadMap()}
            />
          ) : (
            <div
              className={cn(
                'grid gap-3 h-full min-h-[420px]',
                sidePanelOpen
                  ? 'grid-cols-1 xl:grid-cols-[minmax(0,1fr)_320px]'
                  : 'grid-cols-1 xl:grid-cols-[minmax(0,1fr)_48px]',
              )}
            >
              <div className="relative rounded-panel border border-line overflow-hidden bg-surface-1 h-[60vh] xl:h-full min-h-0">
                <ErrorBoundary>
                  <SceneCanvas />
                </ErrorBoundary>
              </div>
              <SidePanel className="h-[60vh] xl:h-full min-h-0 overflow-y-auto" />
            </div>
          )}
        </CapabilityGate>
      </div>
    </div>
  );
}
