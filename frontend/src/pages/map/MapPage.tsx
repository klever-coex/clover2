import { useTranslation } from 'react-i18next';

import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { ErrorBoundary } from '../../components/map/ErrorBoundary.tsx';
import { SidePanel } from '../../components/map/panel/SidePanel.tsx';
import { SceneCanvas } from '../../components/map/scene/SceneCanvas.tsx';
import { ErrorState } from '../../components/common/ErrorState.tsx';
import { LoadingState } from '../../components/common/LoadingState.tsx';
import { cn } from '@/lib/utils';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useMapKeyboardShortcuts } from '@/hooks/useMapKeyboardShortcuts';
import { useCapabilityFetch } from '@/hooks/useCapabilityFetch';
import { useRosCapability } from '@/hooks/useRosCapability';
import { useMapStore } from '@/store/useMapStore';
import { useMapUIStore } from '@/store/useMapUIStore';
import { pageGrid, panelFill } from '@/lib/uiStyles';

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
      <div className="flex-1 min-h-0">
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
                pageGrid,
                sidePanelOpen
                  ? 'grid-cols-1 xl:grid-cols-[minmax(0,1fr)_320px]'
                  : 'grid-cols-1 xl:grid-cols-[minmax(0,1fr)_48px]',
              )}
            >
              <div className="relative rounded-panel border border-border overflow-hidden bg-card">
                <ErrorBoundary>
                  <SceneCanvas />
                </ErrorBoundary>
              </div>
              <SidePanel className={cn(panelFill, 'overflow-y-auto')} />
            </div>
          )}
        </CapabilityGate>
      </div>
    </div>
  );
}
