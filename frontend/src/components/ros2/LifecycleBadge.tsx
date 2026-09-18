import type { LifecycleStateLabel } from '@/types/lifecycle';
import type { TranslationKey } from '@/i18n/index.ts';
import { StateBadge } from '../common/StateBadge.tsx';
import type { badgeVariants } from '@/components/ui/badge';
import type { VariantProps } from 'class-variance-authority';

type BadgeVariant = NonNullable<VariantProps<typeof badgeVariants>['variant']>;

const STATE_VARIANTS: Record<LifecycleStateLabel, BadgeVariant> = {
  unknown: 'secondary', // gray
  unconfigured: 'secondary', // gray
  inactive: 'warning', // yellow: stopped, but not a failure
  active: 'success', // green
  finalized: 'secondary', // gray
  configuring: 'outline', // transition
  cleaningup: 'outline', // transition
  shuttingdown: 'outline', // transition
  activating: 'outline', // transition
  deactivating: 'outline', // transition
  errorprocessing: 'warning', // transition
};

const STATE_KEYS: Record<LifecycleStateLabel, TranslationKey> = {
  unknown: 'nodes.lifecycleState.unknown',
  unconfigured: 'nodes.lifecycleState.unconfigured',
  inactive: 'nodes.lifecycleState.inactive',
  active: 'nodes.lifecycleState.active',
  finalized: 'nodes.lifecycleState.finalized',
  configuring: 'nodes.lifecycleState.configuring',
  cleaningup: 'nodes.lifecycleState.cleaningup',
  shuttingdown: 'nodes.lifecycleState.shuttingdown',
  activating: 'nodes.lifecycleState.activating',
  deactivating: 'nodes.lifecycleState.deactivating',
  errorprocessing: 'nodes.lifecycleState.errorprocessing',
};

interface LifecycleBadgeProps {
  state?: LifecycleStateLabel | null;
}

export function LifecycleBadge({ state }: LifecycleBadgeProps) {
  return (
    <StateBadge
      value={state}
      variantByValue={STATE_VARIANTS}
      labelKeyByValue={STATE_KEYS}
      fallback={{ variant: 'secondary', labelKey: 'nodes.regular' }}
    />
  );
}
