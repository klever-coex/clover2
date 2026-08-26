import { useTranslation } from 'react-i18next';

import type { TranslationKey } from '../../i18n/index.ts';
import type { LifecycleState } from '../../types/node.ts';
import { Badge } from '../ui/Badge.tsx';
import type { BadgeTone } from '../ui/Badge.tsx';

interface LifecycleBadgeProps {
  state?: LifecycleState;
}

const STATE_TONES: Record<LifecycleState, BadgeTone> = {
  unconfigured: 'neutral', // gray
  inactive: 'error', // red
  active: 'success', // green
  finalized: 'neutral', // gray
};

const STATE_KEYS: Record<LifecycleState, TranslationKey> = {
  unconfigured: 'nodes.lifecycleState.unconfigured',
  inactive: 'nodes.lifecycleState.inactive',
  active: 'nodes.lifecycleState.active',
  finalized: 'nodes.lifecycleState.finalized',
};

export function LifecycleBadge({ state }: LifecycleBadgeProps) {
  const { t } = useTranslation();

  if (state !== undefined) {
    return <Badge tone={STATE_TONES[state]}>{t(STATE_KEYS[state])}</Badge>;
  }

  return <Badge tone="neutral">{t('nodes.regular')}</Badge>;
}
