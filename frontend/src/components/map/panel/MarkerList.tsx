import { useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Trash2 } from 'lucide-react';

import { deleteMarkersAfterDetach } from '../../../store/mapMutations.ts';
import { confirmDialog } from '@/store/useConfirmStore';
import { useMapStore } from '@/store/useMapStore';
import { Badge } from '@/components/ui/badge';
import { Card, CardAction, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { CardRow } from '../../common/CardRow.tsx';
import { EmptyState } from '../../common/EmptyState.tsx';
import { ListToolbar } from '../../common/ListToolbar.tsx';
import { SortSelect } from '../../common/SortSelect.tsx';
import { MarkerTypeBadge } from './MarkerTypeBadge.tsx';
import { TooltipButton } from '../../common/TooltipButton.tsx';

export function MarkerList() {
  const { t } = useTranslation();
  const markers = useMapStore((s) => s.markers);
  const selectedMarkerIds = useMapStore((s) => s.selectedMarkerIds);
  const selectMarker = useMapStore((s) => s.selectMarker);
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

  const handleDelete = async (id: string) => {
    const confirmed = await confirmDialog({
      message: t('map.deleteConfirmOne', { id }),
      tone: 'danger',
      confirmLabel: t('map.delete'),
    });
    if (confirmed) {
      deleteMarkersAfterDetach([id]);
    }
  };

  return (
    <Card size="sm" className="min-h-40 flex-1">
      <CardHeader>
        <CardTitle>{t('map.markers')}</CardTitle>
        <CardAction>
          <Badge variant="secondary">{markerList.length}</Badge>
        </CardAction>
      </CardHeader>
      <CardContent className="flex min-h-0 flex-1 flex-col gap-2">
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

        <ul
          className="flex min-h-0 flex-1 flex-col gap-0.5 overflow-y-auto"
          aria-label={t('map.markers')}
        >
          {markerList.map((m) => {
            const key = String(m.id);
            const isSel = selectedMarkerIds.includes(key);
            return (
              <li key={key}>
                <CardRow
                  variant="compact"
                  active={isSel}
                  ariaPressed={isSel}
                  onSelect={(e) => selectMarker(key, e.ctrlKey || e.metaKey)}
                  action={
                    <TooltipButton
                      variant="ghost"
                      size="icon-sm"
                      label={t('map.delete')}
                      onClick={() => void handleDelete(key)}
                    >
                      <Trash2 />
                    </TooltipButton>
                  }
                >
                  <span className="truncate font-mono">
                    #{m.id} <span className="text-muted-foreground/80">{m.markerFrameId}</span>
                  </span>
                  <MarkerTypeBadge type={m.type} />
                </CardRow>
              </li>
            );
          })}
          {markerList.length === 0 && <EmptyState message={t('map.noMarkers')} />}
        </ul>

        {mutationError !== null && (
          <p role="alert" className="text-xs text-destructive">
            {mutationError}
          </p>
        )}
      </CardContent>
    </Card>
  );
}
