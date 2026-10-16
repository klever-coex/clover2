import type { MarkerType } from '@/types/marker';
import type { TranslationKey } from '@/i18n/index.ts';
import { StateBadge } from '../../common/StateBadge.tsx';
import type { badgeVariants } from '@/components/ui/badge';
import type { VariantProps } from 'class-variance-authority';

type BadgeVariant = NonNullable<VariantProps<typeof badgeVariants>['variant']>;

const TYPE_VARIANTS: Record<MarkerType, BadgeVariant> = {
  fixed: 'success',
  static: 'warning',
  dynamic: 'secondary',
};

const TYPE_KEYS: Record<MarkerType, TranslationKey> = {
  fixed: 'map.typeFixed',
  static: 'map.typeStatic',
  dynamic: 'map.typeDynamic',
};

export function MarkerTypeBadge({ type }: { type: MarkerType }) {
  return (
    <StateBadge value={type} variantByValue={TYPE_VARIANTS} labelKeyByValue={TYPE_KEYS} />
  );
}
