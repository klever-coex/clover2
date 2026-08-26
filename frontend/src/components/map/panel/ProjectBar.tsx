import { useTranslation } from 'react-i18next';

import { saveMap } from '../../../store/mapMutations.ts';
import { mapDirtyIds } from '../../../store/slices/mapSlice.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import { Badge } from '../../ui/Badge.tsx';
import { Button } from '../../ui/Button.tsx';

export function ProjectBar() {
  const { t } = useTranslation();
  const mapMeta = useMapStore((s) => s.mapMeta);
  const markerCount = useMapStore((s) => Object.keys(s.markers).length);
  const dirty = useMapStore((s) => mapDirtyIds(s).length > 0);
  const saving = useMapStore((s) => s.saving);
  const mutationError = useMapStore((s) => s.mutationError);

  return (
    <div className="flex flex-col gap-2">
      <div className="flex items-center justify-between gap-2">
        <span className="text-md font-semibold text-ink truncate">
          {mapMeta?.name || t('map.title')}
        </span>
        <div className="flex items-center gap-2 shrink-0">
          <Button
            variant="primary"
            size="md"
            disabled={!dirty || saving}
            onClick={() => void saveMap()}
          >
            {saving ? t('map.saving') : t('map.save')}
          </Button>
        </div>
      </div>
      {mutationError !== null && <p className="text-xs text-error">{mutationError}</p>}
      <div className="flex items-center gap-2 flex-wrap">
        {mapMeta !== null && mapMeta.frameId !== '' && (
          <Badge tone="neutral" className="font-mono">
            {t('map.frame')}: {mapMeta.frameId}
          </Badge>
        )}
        {mapMeta !== null && (
          <Badge tone="neutral" className="font-mono">
            {t('map.dictionary')}: {mapMeta.dictionary}
          </Badge>
        )}
        <Badge tone="neutral">
          {t('map.markers')}: {markerCount}
        </Badge>
      </div>
    </div>
  );
}
