import { Fragment, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { ChevronDown, ChevronRight, Clock } from 'lucide-react';

import { cn } from '@/lib/utils';
import type { DiagnosticLevelName } from '@/types/diagnostic.ts';
import { effectiveLevelValue, LEVEL_NAMES } from '@/types/diagnostic.ts';
import type { DiagnosticTreeNode } from './diagnosticsTree.ts';
import { LevelDot } from './LevelDot.tsx';

interface DiagnosticsTreeProps {
  root: DiagnosticTreeNode;
  now: number;
  staleTimeoutMs: number;
  selectedPath: string | null;
  onSelect: (path: string) => void;
}

interface NodeRowProps {
  node: DiagnosticTreeNode;
  depth: number;
  now: number;
  staleTimeoutMs: number;
  selectedPath: string | null;
  collapsed: Set<string>;
  toggleCollapsed: (path: string) => void;
  onSelect: (path: string) => void;
}

function levelName(levelValue: number): DiagnosticLevelName {
  return LEVEL_NAMES[levelValue] ?? 'stale';
}

function faultCount(node: DiagnosticTreeNode, now: number, staleTimeoutMs: number): number {
  // Leaves count as a fault when their (possibly stale-aged) level is not OK.
  if (node.children.length === 0) {
    return node.entry !== null && effectiveLevelValue(node.entry, now, staleTimeoutMs) > 0 ? 1 : 0;
  }
  const childFaults = node.children.reduce(
    (sum, child) => sum + faultCount(child, now, staleTimeoutMs),
    0,
  );
  // A group's own problem status counts only when it is not derived from children
  // (diagnostic_aggregator group level mirrors the worst child), to avoid double counting.
  if (childFaults === 0 && node.entry !== null && effectiveLevelValue(node.entry, now, staleTimeoutMs) > 0) {
    return 1;
  }
  return childFaults;
}

function NodeRow({
  node,
  depth,
  now,
  staleTimeoutMs,
  selectedPath,
  collapsed,
  toggleCollapsed,
  onSelect,
}: NodeRowProps) {
  const { t } = useTranslation();
  const isGroup = node.children.length > 0;
  const isCollapsed = collapsed.has(node.path);
  const selected = node.path === selectedPath;
  const faults = faultCount(node, now, staleTimeoutMs);

  const level =
    node.entry !== null
      ? effectiveLevelValue(node.entry, now, staleTimeoutMs)
      : node.maxLevelValue;
  const missing =
    node.entry !== null && now - node.entry.updatedAt > staleTimeoutMs;

  const row = (
    <button
      type="button"
      onClick={isGroup ? () => toggleCollapsed(node.path) : () => onSelect(node.path)}
      aria-expanded={isGroup ? !isCollapsed : undefined}
      aria-current={selected ? 'true' : undefined}
      className={cn(
        'flex h-8 w-full items-center gap-2 rounded-row px-2 text-left text-xs transition-colors duration-fast hover:bg-muted/60',
        selected && 'bg-secondary hover:bg-secondary',
        missing && 'opacity-50',
      )}
      style={{ paddingLeft: `${depth * 16 + 8}px` }}
    >
      {isGroup ? (
        isCollapsed ? <ChevronRight className="size-3.5 shrink-0 text-muted-foreground" /> : <ChevronDown className="size-3.5 shrink-0 text-muted-foreground" />
      ) : (
        <span className="size-3.5 shrink-0" />
      )}
      <LevelDot level={levelName(level)} />
      <span className="truncate font-mono">{node.label}</span>
      {missing && <Clock className="size-3 shrink-0 text-muted-foreground" />}
      <span className="ml-auto flex min-w-0 items-center gap-2">
        {node.entry !== null && !missing && (
          <span className="truncate text-muted-foreground">{node.entry.message}</span>
        )}
        {missing && (
          <span className="shrink-0 text-muted-foreground">{t('diagnostics.staleItem')}</span>
        )}
        {faults > 0 && (
          <span className="inline-flex h-4 min-w-4 shrink-0 items-center justify-center rounded-4xl bg-destructive/15 px-1 text-[10px] font-medium text-destructive">
            {faults}
          </span>
        )}
      </span>
    </button>
  );

  return (
    <Fragment>
      {row}
      {isGroup && !isCollapsed && node.children.map((child) => (
        <NodeRow
          key={child.path}
          node={child}
          depth={depth + 1}
          now={now}
          staleTimeoutMs={staleTimeoutMs}
          selectedPath={selectedPath}
          collapsed={collapsed}
          toggleCollapsed={toggleCollapsed}
          onSelect={onSelect}
        />
      ))}
    </Fragment>
  );
}

export function DiagnosticsTree({
  root,
  now,
  staleTimeoutMs,
  selectedPath,
  onSelect,
}: DiagnosticsTreeProps) {
  const [collapsed, setCollapsed] = useState<Set<string>>(new Set());

  const toggleCollapsed = (path: string) => {
    setCollapsed((current) => {
      const next = new Set(current);
      if (next.has(path)) {
        next.delete(path);
      } else {
        next.add(path);
      }
      return next;
    });
  };

  return (
    <div className="flex flex-col">
      {root.children.map((child) => (
        <NodeRow
          key={child.path}
          node={child}
          depth={0}
          now={now}
          staleTimeoutMs={staleTimeoutMs}
          selectedPath={selectedPath}
          collapsed={collapsed}
          toggleCollapsed={toggleCollapsed}
          onSelect={onSelect}
        />
      ))}
    </div>
  );
}
