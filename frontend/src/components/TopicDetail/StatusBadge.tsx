import { useTranslation } from 'react-i18next';
import type { TranslationKey } from '../../i18n/index.ts';
import type { StreamState } from '../../store/slices/streamSlice.ts';

const STATE_STYLES: Record<StreamState, string> = {
  idle: 'bg-gray-200 text-gray-700',
  connecting: 'bg-yellow-100 text-yellow-700',
  connected: 'bg-green-100 text-green-700',
  closed: 'bg-gray-200 text-gray-600',
  error: 'bg-red-100 text-red-700',
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

  return (
    <span className={`text-xs font-semibold px-2 py-1 rounded-full ${STATE_STYLES[state]}`}>
      {t(STATE_LABEL_KEYS[state])}
    </span>
  );
}
