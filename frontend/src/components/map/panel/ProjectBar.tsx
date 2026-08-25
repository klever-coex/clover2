import { useTranslation } from 'react-i18next';

import { downloadProject } from '../../../pages/map/mutations.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import { Badge } from '../../ui/Badge.tsx';
import { Button } from '../../ui/Button.tsx';

/** Map name (read-only), frame/dictionary/count badges, and JSON export. */
export function ProjectBar() {
  const { t } = useTranslation();
  const mapMeta = useMapStore((s) => s.mapMeta);
  const markerCount = useMapStore((s) => Object.keys(s.markers).length);

  return (
    <div className="flex flex-col gap-2">
      <div className="flex items-center justify-between gap-2">
        <span className="text-sm font-semibold text-ink truncate">
          {mapMeta?.name || t('map.title')}
        </span>
        <Button variant="secondary" size="sm" onClick={downloadProject}>
          {t('map.export')}
        </Button>
      </div>
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
