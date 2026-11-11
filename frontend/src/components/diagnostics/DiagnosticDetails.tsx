import { useEffect, useMemo, useRef, useState } from 'react';
import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { Check, Copy, Clock } from 'lucide-react';

import { cn } from '@/lib/utils';
import type { TranslationKey } from '@/i18n/index.ts';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { collectLeafNames, aggregateHistory } from './diagnosticsTree.ts';
import type { DiagnosticTreeNode } from './diagnosticsTree.ts';
import { effectiveLevelValue, LEVEL_NAMES } from '@/types/diagnostic.ts';
import type { DiagnosticEntry, DiagnosticPoint } from '@/types/diagnostic.ts';
import { DIAG_HISTORY_WINDOW_MS } from '@/constants/ros.ts';
import { LevelBadge } from './LevelBadge.tsx';
import { LevelDot } from './LevelDot.tsx';
import { LevelSparkline } from './LevelSparkline.tsx';
import { TooltipButton } from '../common/TooltipButton.tsx';

const SPARK_BUCKETS = 60;

const TAB_KEYS = ['overview', 'values', 'history', 'json'] as const;
type TabKey = (typeof TAB_KEYS)[number];

const TAB_LABEL_KEYS: Record<TabKey, TranslationKey> = {
  overview: 'diagnostics.tabOverview',
  values: 'diagnostics.tabValues',
  history: 'diagnostics.tabHistory',
  json: 'diagnostics.tabJson',
};

interface DiagnosticDetailsProps {
  node: DiagnosticTreeNode;
  history: Record<string, DiagnosticPoint[]>;
  now: number;
  staleTimeoutMs: number;
}

interface FieldRowProps {
  label: string;
  children: ReactNode;
}

function FieldRow({ label, children }: FieldRowProps) {
  return (
    <div className="flex items-start gap-3 py-1.5">
      <span className="w-28 shrink-0 text-xs text-muted-foreground">{label}</span>
      <span className="min-w-0 flex-1 break-words text-xs">{children}</span>
    </div>
  );
}

interface ValuesTableProps {
  values: [string, string][];
}

function ValuesTable({ values }: ValuesTableProps) {
  const { t } = useTranslation();
  if (values.length === 0) {
    return <p className="py-2 text-xs text-muted-foreground">{t('diagnostics.valuesEmpty')}</p>;
  }
  return (
    <div className="overflow-hidden rounded-panel border border-border font-mono text-xs">
      {values.map(([key, value], index) => (
        <div
          key={`${key}-${index}`}
          className={cn(
            'flex items-start gap-3 px-3 py-1.5',
            index % 2 === 1 && 'bg-muted/40',
          )}
        >
          <span className="w-40 shrink-0 break-words text-muted-foreground">{key}</span>
          <span className="min-w-0 flex-1 break-words">{value}</span>
        </div>
      ))}
    </div>
  );
}

function ageSeconds(now: number, updatedAt: number): string {
  return `${Math.max(0, Math.round((now - updatedAt) / 1000))}`;
}

export function DiagnosticDetails({ node, history, now, staleTimeoutMs }: DiagnosticDetailsProps) {
  const { t } = useTranslation();
  const [copied, setCopied] = useState(false);
  const copyTimer = useRef<ReturnType<typeof setTimeout> | null>(null);

  useEffect(
    () => () => {
      if (copyTimer.current !== null) clearTimeout(copyTimer.current);
    },
    [],
  );

  const entry = node.entry;
  const isLeaf = node.children.length === 0;
  const leafNames = useMemo(() => collectLeafNames(node), [node]);
  const missing = entry !== null && now - entry.updatedAt > staleTimeoutMs;

  const sparkBuckets = useMemo(
    () => aggregateHistory(history, leafNames, now, DIAG_HISTORY_WINDOW_MS, SPARK_BUCKETS),
    [history, leafNames, now],
  );

  const entryJson = useMemo(
    () =>
      JSON.stringify(
        entry ?? {
          name: node.path,
          children: leafNames,
        },
        null,
        2,
      ),
    [entry, leafNames, node.path],
  );

  // Badge shows the number of rows rendered in the Values tab: key-value pairs
  // for a leaf, summed pairs across children for a group.
  const valuesBadge = useMemo(() => {
    if (isLeaf) return entry?.values.length ?? 0;
    return leafNames.reduce(
      (sum, name) => sum + (findEntryInNode(node, name)?.values.length ?? 0),
      0,
    );
  }, [entry, isLeaf, leafNames, node]);

  const copyJson = () => {
    void navigator.clipboard.writeText(entryJson).then(() => {
      setCopied(true);
      if (copyTimer.current !== null) clearTimeout(copyTimer.current);
      copyTimer.current = setTimeout(() => setCopied(false), 1500);
    });
  };

  const tabBadge: Partial<Record<TabKey, number>> = {
    values: valuesBadge,
  };

  return (
    <div className="flex h-full min-h-0 flex-col gap-3">
      <div className="flex items-start justify-between gap-2">
        <div className="flex min-w-0 flex-col gap-1">
          <div className="flex items-center gap-2">
            <span className="truncate font-mono text-sm font-medium">{node.path}</span>
            {missing && (
              <span className="inline-flex items-center gap-1 text-xs text-muted-foreground">
                <Clock className="size-3" />
                {t('diagnostics.staleItem')}
              </span>
            )}
          </div>
          <span className="text-xs text-muted-foreground">
            {isLeaf ? t('diagnostics.itemLeaf') : t('diagnostics.itemGroup', { count: leafNames.length })}
          </span>
        </div>
        {entry !== null && (
          <TooltipButton
            variant="ghost"
            size="icon"
            label={copied ? t('diagnostics.copied') : t('diagnostics.copyJson')}
            onClick={copyJson}
          >
            {copied ? <Check className="text-success" /> : <Copy />}
          </TooltipButton>
        )}
      </div>

      <Tabs defaultValue="overview" className="min-h-0 flex-1 gap-3">
        <TabsList aria-label={t('diagnostics.detailsLabel')} className="shrink-0">
          {TAB_KEYS.map((key) => (
            <TabsTrigger key={key} value={key}>
              {t(TAB_LABEL_KEYS[key])}
              {tabBadge[key] !== undefined && tabBadge[key]! > 0 && (
                <span className="rounded-4xl bg-muted px-1.5 text-[10px] font-medium">
                  {tabBadge[key]}
                </span>
              )}
            </TabsTrigger>
          ))}
        </TabsList>

        <TabsContent value="overview" className="min-h-0 overflow-y-auto">
          <OverviewTab node={node} entry={entry} isLeaf={isLeaf} missing={missing} now={now} staleTimeoutMs={staleTimeoutMs} />
        </TabsContent>
        <TabsContent value="values" className="min-h-0 overflow-y-auto">
          <ValuesTab node={node} entry={entry} isLeaf={isLeaf} />
        </TabsContent>
        <TabsContent value="history" className="min-h-0 overflow-y-auto">
          <div className="flex flex-col gap-2">
            <LevelSparkline buckets={sparkBuckets} label={t('diagnostics.historyLabel', { name: node.path })} />
            <p className="text-xs text-muted-foreground">{t('diagnostics.historyHint')}</p>
          </div>
        </TabsContent>
        <TabsContent value="json" className="min-h-0 overflow-y-auto">
          <pre className="overflow-x-auto rounded-panel border border-border bg-muted/40 p-3 font-mono text-xs">
            {entryJson}
          </pre>
        </TabsContent>
      </Tabs>
    </div>
  );
}

interface OverviewTabProps {
  node: DiagnosticTreeNode;
  entry: DiagnosticEntry | null;
  isLeaf: boolean;
  missing: boolean;
  now: number;
  staleTimeoutMs: number;
}

function OverviewTab({ node, entry, isLeaf, missing, now, staleTimeoutMs }: OverviewTabProps) {
  const { t } = useTranslation();

  if (isLeaf) {
    if (entry === null) {
      return <p className="py-2 text-xs text-muted-foreground">{t('diagnostics.noEntryData')}</p>;
    }
    return (
      <div className="flex flex-col divide-y divide-border/60">
        <FieldRow label={t('diagnostics.levelField')}>
          <LevelBadge level={missing ? 'stale' : entry.level} />
        </FieldRow>
        <FieldRow label={t('diagnostics.messageField')}>{entry.message}</FieldRow>
        <FieldRow label={t('diagnostics.hardwareField')}>
          {entry.hardwareId !== '' ? entry.hardwareId : '—'}
        </FieldRow>
        <FieldRow label={t('diagnostics.updatedField')}>
          {missing
            ? t('diagnostics.updatedStale', { seconds: ageSeconds(now, entry.updatedAt) })
            : t('diagnostics.updatedAgo', { seconds: ageSeconds(now, entry.updatedAt) })}
        </FieldRow>
        <FieldRow label={t('diagnostics.valuesField')}>{entry.values.length}</FieldRow>
      </div>
    );
  }

  // Group: aggregate leaf entries.
  const names = collectLeafNames(node);
  const leaves = names
    .map((name) => findEntryInNode(node, name))
    .filter((item): item is DiagnosticEntry => item !== null);
  const counts = { ok: 0, warn: 0, error: 0, stale: 0 };
  for (const leaf of leaves) {
    counts[LEVEL_NAMES[effectiveLevelValue(leaf, now, staleTimeoutMs)] ?? 'stale'] += 1;
  }
  const worst = leaves
    .slice()
    .sort((a, b) => effectiveLevelValue(b, now, staleTimeoutMs) - effectiveLevelValue(a, now, staleTimeoutMs))
    .slice(0, 6);

  return (
    <div className="flex flex-col gap-3">
      <div className="flex flex-wrap items-center gap-2">
        <LevelBadge level="ok" className={counts.ok === 0 ? 'opacity-40' : undefined} />
        <span className="text-xs">{counts.ok}</span>
        <LevelBadge level="warn" className={counts.warn === 0 ? 'opacity-40' : undefined} />
        <span className="text-xs">{counts.warn}</span>
        <LevelBadge level="error" className={counts.error === 0 ? 'opacity-40' : undefined} />
        <span className="text-xs">{counts.error}</span>
        <LevelBadge level="stale" className={counts.stale === 0 ? 'opacity-40' : undefined} />
        <span className="text-xs">{counts.stale}</span>
      </div>
      <div className="flex flex-col">
        <span className="mb-1 text-xs text-muted-foreground">{t('diagnostics.groupWorst')}</span>
        {worst.map((leaf) => (
          <div key={leaf.name} className="flex h-7 items-center gap-2 text-xs">
            <LevelDot level={LEVEL_NAMES[effectiveLevelValue(leaf, now, staleTimeoutMs)] ?? 'stale'} />
            <span className="truncate font-mono">{leaf.name}</span>
            <span className="ml-auto truncate text-muted-foreground">{leaf.message}</span>
          </div>
        ))}
      </div>
    </div>
  );
}

function findEntryInNode(node: DiagnosticTreeNode, name: string): DiagnosticEntry | null {
  if (node.path === name) return node.entry;
  for (const child of node.children) {
    const found = findEntryInNode(child, name);
    if (found !== null) return found;
  }
  return null;
}

interface ValuesTabProps {
  node: DiagnosticTreeNode;
  entry: DiagnosticEntry | null;
  isLeaf: boolean;
}

function ValuesTab({ node, entry, isLeaf }: ValuesTabProps) {
  const { t } = useTranslation();

  if (isLeaf) {
    return <ValuesTable values={entry?.values ?? []} />;
  }

  const names = collectLeafNames(node);
  return (
    <div className="flex flex-col gap-4">
      {names.map((name) => {
        const leaf = findEntryInNode(node, name);
        if (leaf === null || leaf.values.length === 0) return null;
        return (
          <div key={name} className="flex flex-col gap-1">
            <span className="font-mono text-xs text-muted-foreground">{name}</span>
            <ValuesTable values={leaf.values} />
          </div>
        );
      })}
      {names.every((name) => {
        const leaf = findEntryInNode(node, name);
        return leaf === null || leaf.values.length === 0;
      }) && <p className="py-2 text-xs text-muted-foreground">{t('diagnostics.valuesEmpty')}</p>}
    </div>
  );
}
