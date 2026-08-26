import { ChevronRight } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import { cn } from '../../../lib/cn';
import { useMapStore } from '../../../store/useMapStore.ts';
import { useMapUIStore } from '../../../store/useMapUIStore.ts';
import { IconButton } from '../../ui/IconButton.tsx';
import { Panel } from '../../ui/Panel.tsx';
import { AddMarkerButton } from './AddMarkerButton.tsx';
import { MarkerEditor } from './MarkerEditor.tsx';
import { MarkerList } from './MarkerList.tsx';
import { ProjectBar } from './ProjectBar.tsx';

interface SidePanelProps {
  className?: string;
}

export function SidePanel({ className }: SidePanelProps) {
  const { t } = useTranslation();
  const sidePanelOpen = useMapUIStore((s) => s.sidePanelOpen);
  const togglePanel = useMapUIStore((s) => s.togglePanel);
  const selectedIds = useMapStore((s) => s.selectedMarkerIds);
  const markers = useMapStore((s) => s.markers);

  const primaryMarker = selectedIds.length >= 1 ? markers[selectedIds[0]!] : undefined;

  if (!sidePanelOpen) {
    return (
      <div className={className}>
        <IconButton
          icon={ChevronRight}
          label={t('map.togglePanel')}
          expanded={false}
          onClick={togglePanel}
        />
      </div>
    );
  }

  return (
    <div className={cn('flex flex-col gap-3', className)}>
      <div className="flex items-start gap-2">
        <div className="flex-1 min-w-0">
          <ProjectBar />
        </div>
        <IconButton
          icon={ChevronRight}
          label={t('map.togglePanel')}
          onClick={togglePanel}
        />
      </div>
      <AddMarkerButton />
      <MarkerList />
      {primaryMarker !== undefined ? (
        <MarkerEditor marker={primaryMarker} selectedIds={selectedIds} />
      ) : (
        <Panel padded>
          <p className="text-sm text-ink-faint">{t('map.selectHint')}</p>
        </Panel>
      )}
    </div>
  );
}
