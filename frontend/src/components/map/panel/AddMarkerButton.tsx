import { Plus } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import { addMarker } from '../../../store/mapMutations.ts';
import { Button } from '@/components/ui/button';

export function AddMarkerButton() {
  const { t } = useTranslation();

  return (
    <Button onClick={addMarker}>
      <Plus />
      {t('map.addMarker')}
    </Button>
  );
}
