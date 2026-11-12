import { useEffect, useMemo, useState } from 'react';
import { useSearchParams } from 'react-router';
import { useTranslation } from 'react-i18next';
import { Pause, Play } from 'lucide-react';

import { cn } from '@/lib/utils';
import { pageGrid, panelFill } from '@/lib/uiStyles';
import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { StatusBadge } from '../../components/ros2/StatusBadge.tsx';
import { ListToolbar } from '../../components/common/ListToolbar.tsx';
import { SortSelect } from '../../components/common/SortSelect.tsx';
import { TooltipButton } from '../../components/common/TooltipButton.tsx';
import { Button } from '@/components/ui/button';
import { Spinner } from '@/components/ui/spinner';
import { EmptyState } from '../../components/common/EmptyState.tsx';
import { ErrorState } from '../../components/common/ErrorState.tsx';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useRosStore } from '@/store/useRosStore';
import { DIAGNOSTICS_TOPIC, DIAG_HISTORY_WINDOW_MS, DIAG_STALE_TIMEOUT_MS } from '@/constants/ros.ts';
import {
  aggregateHistory,
  buildDiagnosticTree,
  findDiagnosticNode,
} from '../../components/diagnostics/diagnosticsTree.ts';
import { DiagnosticsTree } from '../../components/diagnostics/DiagnosticsTree.tsx';
import { DiagnosticDetails } from '../../components/diagnostics/DiagnosticDetails.tsx';
import { LevelSparkline } from '../../components/diagnostics/LevelSparkline.tsx';
import { LevelBadge } from '../../components/diagnostics/LevelBadge.tsx';
import { effectiveLevelValue, worstLevelValue, LEVEL_NAMES } from '@/types/diagnostic.ts';
import { usePageHeader } from '@/store/usePageHeader';
import { route } from '@/routes/navigation.ts';
import type { TranslationKey } from '@/i18n/index.ts';

type LevelFilter = 'all' | 'warn' | 'error';

const FILTER_MIN_VALUES: Record<LevelFilter, number> = {
  all: 0,
  warn: 1,
  error: 2,
};

/** Client-side staleness maps missing items to this level (DiagnosticStatus.STALE). */
const STALE_LEVEL_VALUE = 3;

const FILTER_LABEL_KEYS: Record<LevelFilter, TranslationKey> = {
  all: 'diagnostics.levelAll',
  warn: 'diagnostics.levelWarnPlus',
  error: 'diagnostics.levelErrorPlus',
};

const SPARK_BUCKETS = 60;

export function DiagnosticsPage() {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();
  const [searchParams, setSearchParams] = useSearchParams();
  const selectedPath = searchParams.get('item');
  const selectItem = (path: string | null) => {
    setSearchParams(
      (current) => {
        if (path === null) {
          current.delete('item');
        } else {
          current.set('item', path);
        }
        return current;
      },
      { replace: true },
    );
  };

  const diagnostics = useRosStore((s) => s.diagnostics);
  const history = useRosStore((s) => s.diagnosticsHistory);
  const diagnosticsState = useRosStore((s) => s.diagnosticsState);
  const diagnosticsMissing = useRosStore((s) => s.diagnosticsMissing);
  const diagnosticsError = useRosStore((s) => s.diagnosticsError);
  const diagnosticsReceived = useRosStore((s) => s.diagnosticsReceived);
  const diagnosticsPaused = useRosStore((s) => s.diagnosticsPaused);
  const diagnosticsDropped = useRosStore((s) => s.diagnosticsDropped);
  const startDiagnosticsStream = useRosStore((s) => s.startDiagnosticsStream);
  const stopDiagnosticsStream = useRosStore((s) => s.stopDiagnosticsStream);
  const retryDiagnosticsStream = useRosStore((s) => s.retryDiagnosticsStream);
  const clearDiagnostics = useRosStore((s) => s.clearDiagnostics);
  const setDiagnosticsPaused = useRosStore((s) => s.setDiagnosticsPaused);

  useEffect(() => {
    startDiagnosticsStream();
    return () => stopDiagnosticsStream();
  }, [startDiagnosticsStream, stopDiagnosticsStream]);

  const [query, setQuery] = useState('');
  const [minLevel, setMinLevel] = useState<LevelFilter>('all');

  usePageHeader(
    selectedPath !== null
      ? [
          { label: t(route('/diagnostics').labelKey), to: '/diagnostics' },
          { label: selectedPath, mono: true },
        ]
      : [],
  );

  // Ticks once per second so ages and client-side staleness refresh.
  // While paused the tick stops: the frozen view must not age into "no updates".
  // After resume the next tick catches up within a second — no need for an
  // eager update here.
  const [now, setNow] = useState(() => Date.now());
  useEffect(() => {
    if (diagnosticsPaused) return;
    const id = setInterval(() => setNow(Date.now()), 1000);
    return () => clearInterval(id);
  }, [diagnosticsPaused]);

  const visibleEntries = useMemo(() => {
    const q = query.trim().toLowerCase();
    return Object.values(diagnostics).filter((entry) => {
      const level = effectiveLevelValue(entry, now, DIAG_STALE_TIMEOUT_MS);
      // Items with no updates are only visible unfiltered: "Warn+" and "Error+"
      // describe published levels, not missing data.
      if (minLevel !== 'all' && level >= STALE_LEVEL_VALUE) return false;
      if (minLevel !== 'all' && entry.levelValue < FILTER_MIN_VALUES[minLevel]) return false;
      if (q === '') return true;
      return (
        entry.name.toLowerCase().includes(q) ||
        entry.message.toLowerCase().includes(q) ||
        entry.hardwareId.toLowerCase().includes(q)
      );
    });
  }, [diagnostics, minLevel, now, query]);

  const tree = useMemo(() => buildDiagnosticTree(visibleEntries), [visibleEntries]);
  const selectedNode = findDiagnosticNode(tree, selectedPath);

  const counts = useMemo(() => {
    const result = { ok: 0, warn: 0, error: 0, stale: 0 };
    for (const entry of Object.values(diagnostics)) {
      result[LEVEL_NAMES[effectiveLevelValue(entry, now, DIAG_STALE_TIMEOUT_MS)] ?? 'stale'] += 1;
    }
    return result;
  }, [diagnostics, now]);

  const overall = LEVEL_NAMES[Math.max(0, worstLevelValue(Object.values(diagnostics).map((entry) => effectiveLevelValue(entry, now, DIAG_STALE_TIMEOUT_MS))))] ?? 'stale';

  const sparkBuckets = useMemo(() => {
    const names = Object.keys(diagnostics);
    return aggregateHistory(history, names, now, DIAG_HISTORY_WINDOW_MS, SPARK_BUCKETS);
  }, [diagnostics, history, now]);

  const filterOptions = useMemo(
    () =>
      (Object.keys(FILTER_MIN_VALUES) as LevelFilter[]).map((level) => ({
        value: level,
        label: t(FILTER_LABEL_KEYS[level]),
      })),
    [t],
  );

  const connecting = diagnosticsState === 'idle' || diagnosticsState === 'connecting';
  const hasError = diagnosticsError !== null && diagnosticsState !== 'connected';
  const isEmpty = Object.keys(diagnostics).length === 0;

  return (
    <div className="p-6 h-full flex flex-col">
      <CapabilityGate name="topics" noCapability={t('diagnostics.noCapability')}>
        <div className="flex-1 min-h-0 flex flex-col gap-3">
          <div className="flex items-center gap-3 flex-wrap">
            <StatusBadge state={diagnosticsState} />
            <span className="text-xs text-muted-foreground">
              {diagnosticsReceived} {t('diagnostics.received')}
            </span>
            {diagnosticsPaused && diagnosticsDropped > 0 && (
              <span className="text-xs text-warning">
                {t('diagnostics.pausedBadge', { count: diagnosticsDropped })}
              </span>
            )}
          </div>

          <ListToolbar
            value={query}
            onChange={setQuery}
            placeholder={t('diagnostics.searchPlaceholder')}
            extra={
              <>
                <SortSelect
                  ariaLabel={t('diagnostics.minLevel')}
                  value={minLevel}
                  onChange={(value) => setMinLevel(value as LevelFilter)}
                  options={filterOptions}
                />
                <TooltipButton
                  variant="ghost"
                  size="icon"
                  label={diagnosticsPaused ? t('diagnostics.resume') : t('diagnostics.pause')}
                  onClick={() => setDiagnosticsPaused(!diagnosticsPaused)}
                >
                  {diagnosticsPaused ? <Play /> : <Pause />}
                </TooltipButton>
                <Button variant="secondary" size="sm" onClick={clearDiagnostics}>
                  {t('diagnostics.clear')}
                </Button>
              </>
            }
          />

          {!isEmpty && (
            <div className="flex flex-wrap items-center gap-x-4 gap-y-2">
              <div className="flex items-center gap-2">
                <span className="text-xs text-muted-foreground">{t('diagnostics.overall')}</span>
                <LevelBadge level={overall} />
              </div>
              <div className="flex items-center gap-3 text-xs">
                <span className="text-success">{t('diagnostics.levelOk')}: {counts.ok}</span>
                <span className="text-warning">{t('diagnostics.levelWarn')}: {counts.warn}</span>
                <span className="text-destructive">{t('diagnostics.levelError')}: {counts.error}</span>
                <span className="text-muted-foreground">{t('diagnostics.levelStale')}: {counts.stale}</span>
              </div>
              <LevelSparkline
                buckets={sparkBuckets}
                label={t('diagnostics.historyLabel', { name: '/diagnostics_agg' })}
                className="w-48"
              />
            </div>
          )}

          {isEmpty ? (
            <div className="flex-1 flex items-center justify-center">
              {diagnosticsMissing ? (
                <ErrorState
                  message={t('diagnostics.topicMissing', { topic: DIAGNOSTICS_TOPIC })}
                  onRetry={retryDiagnosticsStream}
                />
              ) : connecting && !hasError ? (
                <Spinner className="size-8 text-primary" />
              ) : hasError && diagnosticsError !== null ? (
                <ErrorState
                  message={errorMessage(diagnosticsError)}
                  onRetry={retryDiagnosticsStream}
                />
              ) : (
                <EmptyState message={t('diagnostics.empty')} />
              )}
            </div>
          ) : (
            <div className={cn(pageGrid, 'flex-1 min-h-0 grid-cols-1 xl:grid-cols-[minmax(300px,400px)_minmax(0,1fr)]')}>
              <div className="min-h-0 overflow-y-auto rounded-panel border border-border bg-card p-2 h-[30vh] xl:h-full">
                <DiagnosticsTree
                  root={tree}
                  now={now}
                  staleTimeoutMs={DIAG_STALE_TIMEOUT_MS}
                  selectedPath={selectedPath}
                  onSelect={selectItem}
                />
                {tree.children.length === 0 && <EmptyState message={t('diagnostics.noMatches')} />}
              </div>
              <div className={cn(panelFill, 'relative overflow-y-auto rounded-panel border border-border bg-card p-4')}>
                {selectedNode !== null ? (
                  <DiagnosticDetails
                    node={selectedNode}
                    history={history}
                    now={now}
                    staleTimeoutMs={DIAG_STALE_TIMEOUT_MS}
                  />
                ) : (
                  <div className="flex h-full items-center justify-center">
                    <p className="text-sm text-muted-foreground">{t('diagnostics.selectHint')}</p>
                  </div>
                )}
              </div>
            </div>
          )}

          {!isEmpty && hasError && diagnosticsError !== null && (
            <p role="alert" className="text-xs text-destructive">
              {diagnosticsMissing
                ? t('diagnostics.topicMissing', { topic: DIAGNOSTICS_TOPIC })
                : errorMessage(diagnosticsError)}
            </p>
          )}
        </div>
      </CapabilityGate>
    </div>
  );
}

export default DiagnosticsPage;
