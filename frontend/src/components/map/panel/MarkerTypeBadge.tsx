import { useTranslation } from 'react-i18next';

import type { TranslationKey } from '../../../i18n/index.ts';
import type { MarkerType } from '../../../types/marker.ts';
import { Badge } from '../../ui/Badge.tsx';
import type { BadgeTone } from '../../ui/Badge.tsx';

const TYPE_TONES: Record<MarkerType, BadgeTone> = {
  fixed: 'accent',
  static: 'neutral',
  dynamic: 'warning',
};

const TYPE_KEYS: Record<MarkerType, TranslationKey> = {
  fixed: 'map.typeFixed',
  static: 'map.typeStatic',
  dynamic: 'map.typeDynamic',
};

export function MarkerTypeBadge({ type }: { type: MarkerType }) {
  const { t } = useTranslation();

  return <Badge tone={TYPE_TONES[type]}>{t(TYPE_KEYS[type])}</Badge>;
}
