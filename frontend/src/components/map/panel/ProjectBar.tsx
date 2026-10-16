import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';

import { saveMap } from '../../../store/mapMutations.ts';
import { mapDirtyIds } from '../../../store/slices/mapSlice.ts';
import { useMapStore } from '@/store/useMapStore';
import { Badge } from '@/components/ui/badge';
import { Button } from '@/components/ui/button';
import { Card, CardAction, CardContent, CardHeader, CardTitle } from '@/components/ui/card';

interface ProjectBarProps {
  action?: ReactNode;
}

export function ProjectBar({ action }: ProjectBarProps) {
  const { t } = useTranslation();
  const mapMeta = useMapStore((s) => s.mapMeta);
  const markerCount = useMapStore((s) => Object.keys(s.markers).length);
  const dirty = useMapStore((s) => mapDirtyIds(s).length > 0);
  const saving = useMapStore((s) => s.saving);
  const mutationError = useMapStore((s) => s.mutationError);

  return (
    <Card size="sm">
      <CardHeader>
        <CardTitle className="truncate">
          {mapMeta?.name || t('map.title')}
        </CardTitle>
        <CardAction className="flex items-center gap-1">
          {/* Secondary: the panel's accent action is "add marker"; keep one accent per view. */}
          <Button
            variant="secondary"
            size="sm"
            disabled={!dirty || saving}
            onClick={() => void saveMap()}
          >
            {saving ? t('map.saving') : t('map.save')}
          </Button>
          {action}
        </CardAction>
      </CardHeader>
      <CardContent className="flex flex-col gap-2">
        {mutationError !== null && <p className="text-xs text-destructive">{mutationError}</p>}
        <div className="flex flex-wrap items-center gap-1.5">
          {mapMeta !== null && mapMeta.frameId !== '' && (
            <Badge variant="secondary" className="font-mono">
              {t('map.frame')}: {mapMeta.frameId}
            </Badge>
          )}
          {mapMeta !== null && (
            <Badge variant="secondary" className="font-mono">
              {t('map.dictionary')}: {mapMeta.dictionary}
            </Badge>
          )}
          <Badge variant="secondary">
            {t('map.markers')}: {markerCount}
          </Badge>
        </div>
      </CardContent>
    </Card>
  );
}
