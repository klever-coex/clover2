import type { StreamState } from '../../store/slices/streamSlice.ts';
import type { TranslationKey } from '@/i18n/index.ts';
import { StateBadge } from '../common/StateBadge.tsx';
import type { badgeVariants } from '@/components/ui/badge';
import type { VariantProps } from 'class-variance-authority';

type BadgeVariant = NonNullable<VariantProps<typeof badgeVariants>['variant']>;

const STATE_VARIANTS: Record<StreamState, BadgeVariant> = {
  idle: 'secondary',
  connecting: 'warning',
  connected: 'success',
  closed: 'secondary',
  error: 'destructive',
};

const STATE_LABEL_KEYS: Record<StreamState, TranslationKey> = {
  idle: 'topicDetail.connecting',
  connecting: 'topicDetail.connecting',
  connected: 'topicDetail.connected',
  closed: 'topicDetail.closed',
  error: 'topicDetail.error',
};

export function StatusBadge({ state }: { state: StreamState }) {
  return (
    <StateBadge
      value={state}
      variantByValue={STATE_VARIANTS}
      labelKeyByValue={STATE_LABEL_KEYS}
    />
  );
}
