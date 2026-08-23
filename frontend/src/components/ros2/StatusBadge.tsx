import { useTranslation } from 'react-i18next';
import type { TranslationKey } from '../../i18n/index.ts';
import type { StreamState } from '../../store/slices/streamSlice.ts';
import { Badge } from '../ui/Badge.tsx';
import type { BadgeTone } from '../ui/Badge.tsx';

const STATE_TONES: Record<StreamState, BadgeTone> = {
  idle: 'neutral',
  connecting: 'warning',
  connected: 'success',
  closed: 'neutral',
  error: 'error',
};

const STATE_LABEL_KEYS: Record<StreamState, TranslationKey> = {
  idle: 'topicDetail.connecting',
  connecting: 'topicDetail.connecting',
  connected: 'topicDetail.connected',
  closed: 'topicDetail.closed',
  error: 'topicDetail.error',
};

export function StatusBadge({ state }: { state: StreamState }) {
  const { t } = useTranslation();

  return <Badge tone={STATE_TONES[state]}>{t(STATE_LABEL_KEYS[state])}</Badge>;
}
