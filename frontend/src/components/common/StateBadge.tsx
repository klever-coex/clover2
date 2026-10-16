import { useTranslation } from 'react-i18next';
import type { VariantProps } from 'class-variance-authority';

import type { TranslationKey } from '@/i18n/index.ts';
import { cn } from '@/lib/utils';
import { Badge, type badgeVariants } from '@/components/ui/badge';

type BadgeVariant = NonNullable<VariantProps<typeof badgeVariants>['variant']>;

interface StateBadgeProps<T extends string> {
  value: T | null | undefined;
  variantByValue: Record<T, BadgeVariant>;
  labelKeyByValue: Record<T, TranslationKey>;
  fallback?: { variant: BadgeVariant; labelKey: TranslationKey };
  mono?: boolean;
  className?: string;
}

export function StateBadge<T extends string>({
  value,
  variantByValue,
  labelKeyByValue,
  fallback,
  mono,
  className,
}: StateBadgeProps<T>) {
  const { t } = useTranslation();

  const variant = value !== null && value !== undefined
    ? variantByValue[value]
    : fallback?.variant;
  const labelKey = value !== null && value !== undefined
    ? labelKeyByValue[value]
    : fallback?.labelKey;

  if (variant === undefined || labelKey === undefined) return null;

  return (
    <Badge variant={variant} className={cn(mono && 'font-mono', className)}>
      {t(labelKey)}
    </Badge>
  );
}
