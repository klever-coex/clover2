import { useEffect, useLayoutEffect, useMemo, useRef, useState } from 'react';
import { Pause, Play, SearchIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import { CapabilityGate } from '../../components/ros2/CapabilityGate.tsx';
import { LogRow } from '../../components/ros2/LogRow.tsx';
import { StatusBadge } from '../../components/ros2/StatusBadge.tsx';
import { TooltipButton } from '../../components/common/TooltipButton.tsx';
import { Button } from '@/components/ui/button';
import { InputGroup, InputGroupAddon, InputGroupInput } from '@/components/ui/input-group';
import {
  Select,
  SelectContent,
  SelectGroup,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { Spinner } from '@/components/ui/spinner';
import { EmptyState } from '../../components/common/EmptyState.tsx';
import { ErrorState } from '../../components/common/ErrorState.tsx';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useRosCapability } from '@/hooks/useRosCapability';
import { useRosStore } from '@/store/useRosStore';
import type { TranslationKey } from '@/i18n/index.ts';

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

  // Fresh container (first mount or after Clear) always follows new messages.
  useEffect(() => {
    stickToBottomRef.current = true;
  }, [logs.length === 0]);

  useLayoutEffect(() => {
    const el = scrollRef.current;
    if (el !== null && stickToBottomRef.current && !logsPaused) {
      el.scrollTop = el.scrollHeight;
    }
  }, [visible, logsPaused]);

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
      <div className="flex-1 min-h-0 flex flex-col gap-3">
        <CapabilityGate capability={capability} noCapability={t('logs.noCapability')}>
          <div className="flex items-center gap-3 flex-wrap">
            <StatusBadge state={logsState} />
            <span className="text-xs text-muted-foreground">
              {logsReceived} {t('logs.received')}
            </span>
            {logsPaused && logsDropped > 0 && (
              <span className="text-xs text-warning">
                {t('logs.pausedBadge', { count: logsDropped })}
              </span>
            )}
          </div>

          <div className="flex items-center gap-3 flex-wrap">
            <InputGroup className="w-full flex-1 min-w-48">
              <InputGroupAddon align="inline-start">
                <SearchIcon />
              </InputGroupAddon>
              <InputGroupInput
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                placeholder={t('logs.searchPlaceholder')}
                aria-label={t('logs.searchPlaceholder')}
              />
            </InputGroup>
            <Select
              value={minLevel}
              onValueChange={(value) => setMinLevel(value as LevelFilter)}
            >
              <SelectTrigger aria-label={t('logs.minLevel')} className="w-36">
                <SelectValue />
              </SelectTrigger>
              <SelectContent side="bottom">
                <SelectGroup>
                  {levelOptions.map((option) => (
                    <SelectItem key={option.value} value={option.value}>
                      {option.label}
                    </SelectItem>
                  ))}
                </SelectGroup>
              </SelectContent>
            </Select>
            <TooltipButton
              variant="ghost"
              size="icon"
              label={logsPaused ? t('logs.resume') : t('logs.pause')}
              onClick={() => setLogsPaused(!logsPaused)}
            >
              {logsPaused ? <Play /> : <Pause />}
            </TooltipButton>
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
              className="min-h-0 flex-1 overflow-y-auto rounded-panel border border-border bg-card font-mono"
            >
              {visible.length === 0 ? (
                <EmptyState message={t('logs.noMatches')} />
              ) : (
                visible.map((entry) => <LogRow key={entry.seq} entry={entry} />)
              )}
            </div>
          )}

          {logs.length > 0 && hasError && logsError !== null && (
            <p role="alert" className="text-xs text-destructive">{errorMessage(logsError)}</p>
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
        <Spinner className="size-8 text-primary" />
      </div>
    );
  }
  if (hasError && errorMessage !== null) {
    return <ErrorState message={errorMessage} onRetry={onRetry} />;
  }
  return <EmptyState message={emptyMessage} />;
}
