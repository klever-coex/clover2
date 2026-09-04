import { useEffect, useMemo, useRef, useState } from 'react';
import { Pause, Play } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { LogRow } from '../../components/ros2/LogRow.tsx';
import { StatusBadge } from '../../components/ros2/StatusBadge.tsx';
import { Button } from '../../components/ui/Button.tsx';
import { EmptyState } from '../../components/ui/EmptyState.tsx';
import { ErrorState } from '../../components/ui/ErrorState.tsx';
import { IconButton } from '../../components/ui/IconButton.tsx';
import { PageHeader } from '../../components/ui/PageHeader.tsx';
import { SearchInput } from '../../components/ui/SearchInput.tsx';
import { Select } from '../../components/ui/Select.tsx';
import { Spinner } from '../../components/ui/Spinner.tsx';
import { useApiErrorMessage } from '../../hooks/useApiErrorMessage.ts';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useRosStore } from '../../store/useRosStore.ts';
import type { TranslationKey } from '../../i18n/index.ts';

type LevelFilter = 'debug' | 'info' | 'warn' | 'error' | 'fatal';

const LEVEL_VALUES: Record<LevelFilter, number> = {
  debug: 10,
  info: 20,
  warn: 30,
  error: 40,
  fatal: 50,
};

const LEVEL_LABEL_KEYS: Record<LevelFilter, TranslationKey> = {
  debug: 'logs.levelAll',
  info: 'logs.levelInfo',
  warn: 'logs.levelWarn',
  error: 'logs.levelError',
  fatal: 'logs.levelFatal',
};

const AUTOSCROLL_TOLERANCE_PX = 40;

export function LogsPage() {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();
  const capability = useRosCapability('topics');

  const logs = useRosStore((s) => s.logs);
  const logsState = useRosStore((s) => s.logsState);
  const logsError = useRosStore((s) => s.logsError);
  const logsReceived = useRosStore((s) => s.logsReceived);
  const logsPaused = useRosStore((s) => s.logsPaused);
  const logsDropped = useRosStore((s) => s.logsDropped);
  const setLogsPaused = useRosStore((s) => s.setLogsPaused);
  const clearLogs = useRosStore((s) => s.clearLogs);
  const retryRosoutStream = useRosStore((s) => s.retryRosoutStream);

  const [query, setQuery] = useState('');
  const [minLevel, setMinLevel] = useState<LevelFilter>('debug');

  const q = query.trim().toLowerCase();
  const minValue = LEVEL_VALUES[minLevel];
  const visible = useMemo(
    () =>
      logs.filter(
        (entry) =>
          entry.levelValue >= minValue &&
          (q === '' ||
            entry.msg.toLowerCase().includes(q) ||
            entry.name.toLowerCase().includes(q) ||
            (entry.file !== undefined && entry.file.toLowerCase().includes(q))),
      ),
    [logs, minValue, q],
  );

  const scrollRef = useRef<HTMLDivElement>(null);
  const stickToBottomRef = useRef(true);

  const handleScroll = () => {
    const el = scrollRef.current;
    if (el === null) return;
    stickToBottomRef.current =
      el.scrollHeight - el.scrollTop - el.clientHeight < AUTOSCROLL_TOLERANCE_PX;
  };

  useEffect(() => {
    const el = scrollRef.current;
    if (el !== null && stickToBottomRef.current && !logsPaused) {
      el.scrollTop = el.scrollHeight;
    }
  }, [visible.length, logsPaused]);

  const levelOptions = useMemo(
    () =>
      (Object.keys(LEVEL_VALUES) as LevelFilter[]).map((level) => ({
        value: level,
        label: t(LEVEL_LABEL_KEYS[level]),
      })),
    [t],
  );

  const connecting = logsState === 'idle' || logsState === 'connecting';
  const hasError = logsError !== null && logsState !== 'connected';

  return (
    <div className="p-6 h-full flex flex-col">
      <PageHeader title={t('logs.title')} />

      <div className="mt-4 flex-1 min-h-0 flex flex-col gap-3">
        <CapabilityGate capability={capability} noCapability={t('logs.noCapability')}>
          <div className="flex items-center gap-3 flex-wrap">
            <StatusBadge state={logsState} />
            <span className="text-xs text-ink-muted">
              {logsReceived} {t('logs.received')}
            </span>
            {logsPaused && logsDropped > 0 && (
              <span className="text-xs text-warning">
                {t('logs.pausedBadge', { count: logsDropped })}
              </span>
            )}
          </div>

          <div className="flex items-center gap-3 flex-wrap">
            <SearchInput
              value={query}
              onChange={(e) => setQuery(e.target.value)}
              placeholder={t('logs.searchPlaceholder')}
              aria-label={t('logs.searchPlaceholder')}
              className="flex-1 min-w-48"
            />
            <Select
              aria-label={t('logs.minLevel')}
              value={minLevel}
              onChange={(value) => setMinLevel(value as LevelFilter)}
              options={levelOptions}
              className="w-36"
            />
            <IconButton
              icon={logsPaused ? Play : Pause}
              label={logsPaused ? t('logs.resume') : t('logs.pause')}
              onClick={() => setLogsPaused(!logsPaused)}
            />
            <Button variant="secondary" size="sm" onClick={clearLogs}>
              {t('logs.clear')}
            </Button>
          </div>

          {logs.length === 0 ? (
            <LogsPlaceholder
              connecting={connecting && logsError === null}
              hasError={hasError}
              errorMessage={logsError !== null ? errorMessage(logsError) : null}
              onRetry={retryRosoutStream}
              emptyMessage={t('logs.empty')}
            />
          ) : (
            <div
              ref={scrollRef}
              onScroll={handleScroll}
              className="flex-1 min-h-[300px] overflow-y-auto rounded-panel border border-line bg-surface-1 font-mono"
            >
              {visible.length === 0 ? (
                <EmptyState message={t('logs.noMatches')} />
              ) : (
                visible.map((entry) => <LogRow key={entry.seq} entry={entry} />)
              )}
            </div>
          )}

          {logs.length > 0 && hasError && logsError !== null && (
            <p role="alert" className="text-xs text-error">{errorMessage(logsError)}</p>
          )}
        </CapabilityGate>
      </div>
    </div>
  );
}

interface LogsPlaceholderProps {
  connecting: boolean;
  hasError: boolean;
  errorMessage: string | null;
  onRetry: () => void;
  emptyMessage: string;
}

function LogsPlaceholder({
  connecting,
  hasError,
  errorMessage,
  onRetry,
  emptyMessage,
}: LogsPlaceholderProps) {
  if (connecting) {
    return (
      <div className="flex-1 flex items-center justify-center">
        <Spinner size={32} />
      </div>
    );
  }
  if (hasError && errorMessage !== null) {
    return <ErrorState message={errorMessage} onRetry={onRetry} />;
  }
  return <EmptyState message={emptyMessage} />;
}
