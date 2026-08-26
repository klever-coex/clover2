import { useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { cn } from '../../../lib/cn';
import { deleteMarkers } from '../../../store/mapMutations.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import { EmptyState } from '../../ui/EmptyState.tsx';
import { ListToolbar } from '../../ros2/ListToolbar.tsx';
import { SortSelect } from '../../ros2/SortSelect.tsx';
import { MarkerTypeBadge } from './MarkerTypeBadge.tsx';

export function MarkerList() {
  const { t } = useTranslation();
  const markers = useMapStore((s) => s.markers);
  const selectedMarkerIds = useMapStore((s) => s.selectedMarkerIds);
  const selectMarker = useMapStore((s) => s.selectMarker);
  const deselectAll = useMapStore((s) => s.deselectAll);
  const mutationError = useMapStore((s) => s.mutationError);

  const [search, setSearch] = useState('');
  const [sortBy, setSortBy] = useState<'id' | 'type' | 'label'>('id');

  const markerList = useMemo(() => {
    let list = Object.values(markers);
    // Filter
    if (search) {
      const q = search.toLowerCase();
      list = list.filter(
        (m) =>
          m.markerFrameId.toLowerCase().includes(q) ||
          String(m.id).includes(q) ||
          m.type.toLowerCase().includes(q),
      );
    }
    // Sort
    list.sort((a, b) => {
      switch (sortBy) {
        case 'id': return a.id - b.id;
        case 'type': return a.type.localeCompare(b.type);
        case 'label': return a.markerFrameId.localeCompare(b.markerFrameId);
        default: return 0;
      }
    });
    return list;
  }, [markers, search, sortBy]);

  const handleDelete = (id: string) => {
    if (confirm(t('map.deleteConfirmOne', { id }))) {
      deselectAll();
      // Small delay to let TransformControls detach
      requestAnimationFrame(() => void deleteMarkers([id]));
    }
  };

  return (
    <div className="flex flex-col gap-2 min-h-0">
      <div className="text-xs font-semibold text-ink-muted">
        {t('map.markers')} ({markerList.length})
      </div>

      <ListToolbar
        value={search}
        onChange={setSearch}
        placeholder={t('map.searchPlaceholder')}
        extra={
          <SortSelect
            ariaLabel={t('common.sortBy')}
            value={sortBy}
            onChange={(value) => setSortBy(value as 'id' | 'type' | 'label')}
            options={[
              { value: 'id', label: t('map.sortId') },
              { value: 'type', label: t('map.sortType') },
              { value: 'label', label: t('map.sortLabel') },
            ]}
          />
        }
      />

      <div className="min-h-0 overflow-y-auto space-y-0.5">
        {markerList.map((m) => {
          const key = String(m.id);
          const isSel = selectedMarkerIds.includes(key);
          return (
            <button
              key={key}
              type="button"
              onClick={(e) => selectMarker(key, e.ctrlKey || e.metaKey)}
              onDoubleClick={() => handleDelete(key)}
              className={cn(
                'w-full flex items-center justify-between gap-2 px-2 py-1 rounded-row text-left text-xs transition-colors duration-fast cursor-pointer',
                isSel ? 'bg-surface-3 text-ink' : 'text-ink-muted hover:bg-surface-2 hover:text-ink',
              )}
            >
              <span className="truncate font-mono">
                #{m.id} <span className="text-ink-faint">{m.markerFrameId}</span>
              </span>
              <MarkerTypeBadge type={m.type} />
            </button>
          );
        })}
        {markerList.length === 0 && <EmptyState message={t('map.noMarkers')} />}
      </div>

      {mutationError !== null && <p className="text-xs text-error">{mutationError}</p>}
    </div>
  );
}
