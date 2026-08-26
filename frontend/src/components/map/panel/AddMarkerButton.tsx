import { Plus } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import { addMarker } from '../../../store/mapMutations.ts';
import { Button } from '../../ui/Button.tsx';

export function AddMarkerButton() {
  const { t } = useTranslation();

  return (
    <Button onClick={addMarker}>
      <Plus size={16} />
      {t('map.addMarker')}
    </Button>
  );
}
