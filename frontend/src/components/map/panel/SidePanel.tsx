import { ChevronRight } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import { cn } from '@/lib/utils';
import { useMapStore } from '@/store/useMapStore';
import { useMapUIStore } from '@/store/useMapUIStore';
import { AddMarkerButton } from './AddMarkerButton.tsx';
import { EmptyState } from '../../common/EmptyState.tsx';
import { MarkerEditor } from './MarkerEditor.tsx';
import { MarkerList } from './MarkerList.tsx';
import { ProjectBar } from './ProjectBar.tsx';
import { TooltipButton } from '../../common/TooltipButton.tsx';

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
        <TooltipButton
          variant="ghost"
          size="icon"
          label={t('map.togglePanel')}
          aria-expanded={false}
          onClick={togglePanel}
        >
          <ChevronRight />
        </TooltipButton>
      </div>
    );
  }

  return (
    <div className={cn('flex min-h-0 flex-col gap-3 overflow-y-auto', className)}>
      <ProjectBar
        action={
          <TooltipButton
            variant="ghost"
            size="icon-sm"
            label={t('map.togglePanel')}
            aria-expanded
            onClick={togglePanel}
          >
            <ChevronRight />
          </TooltipButton>
        }
      />
      <AddMarkerButton />
      <MarkerList />
      {primaryMarker !== undefined ? (
        <MarkerEditor
          key={String(primaryMarker.id)}
          marker={primaryMarker}
          selectedIds={selectedIds}
        />
      ) : (
        <EmptyState message={t('map.selectHint')} />
      )}
    </div>
  );
}
