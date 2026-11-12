import type { TranslationKey } from '@/i18n/index.ts';
import type { DiagnosticLevelName } from '@/types/diagnostic.ts';
import type { badgeVariants } from '@/components/ui/badge';
import type { VariantProps } from 'class-variance-authority';
import { StateBadge } from '../common/StateBadge.tsx';

type BadgeVariant = NonNullable<VariantProps<typeof badgeVariants>['variant']>;

const LEVEL_VARIANTS: Record<DiagnosticLevelName, BadgeVariant> = {
  ok: 'success',
  warn: 'warning',
  error: 'destructive',
  stale: 'secondary',
};

const LEVEL_LABEL_KEYS: Record<DiagnosticLevelName, TranslationKey> = {
  ok: 'diagnostics.levelOk',
  warn: 'diagnostics.levelWarn',
  error: 'diagnostics.levelError',
  stale: 'diagnostics.levelStale',
};

interface LevelBadgeProps {
  level: DiagnosticLevelName;
  className?: string;
}

export function LevelBadge({ level, className }: LevelBadgeProps) {
  return (
    <StateBadge value={level} variantByValue={LEVEL_VARIANTS} labelKeyByValue={LEVEL_LABEL_KEYS} className={className} />
  );
}
