import type { LifecycleState } from '@/types/node';
import type { TranslationKey } from '@/i18n/index.ts';
import { StateBadge } from '../common/StateBadge.tsx';
import type { badgeVariants } from '@/components/ui/badge';
import type { VariantProps } from 'class-variance-authority';

type BadgeVariant = NonNullable<VariantProps<typeof badgeVariants>['variant']>;

const STATE_VARIANTS: Record<LifecycleState, BadgeVariant> = {
  unconfigured: 'secondary', // gray
  inactive: 'destructive', // red
  active: 'success', // green
  finalized: 'secondary', // gray
};

const STATE_KEYS: Record<LifecycleState, TranslationKey> = {
  unconfigured: 'nodes.lifecycleState.unconfigured',
  inactive: 'nodes.lifecycleState.inactive',
  active: 'nodes.lifecycleState.active',
  finalized: 'nodes.lifecycleState.finalized',
};

interface LifecycleBadgeProps {
  state?: LifecycleState | null;
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
