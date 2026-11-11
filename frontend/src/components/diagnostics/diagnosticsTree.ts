import type { DiagnosticEntry, DiagnosticPoint } from '@/types/diagnostic.ts';

export interface DiagnosticTreeNode {
  /** Path segment shown in the tree, e.g. "imu". */
  label: string;
  /** Full aggregated name, e.g. "/sensors/imu". Always has a leading "/". */
  path: string;
  /** Entry published for this exact path (diagnostic_aggregator also publishes group items). */
  entry: DiagnosticEntry | null;
  children: DiagnosticTreeNode[];
  /** Worst level value within the subtree (entry levels only, -1 when empty). */
  maxLevelValue: number;
}

function normalizeName(name: string): string[] {
  const segments = name.split('/').filter((segment) => segment !== '');
  return segments;
}

function ensureNode(
  root: DiagnosticTreeNode,
  segments: string[],
): DiagnosticTreeNode {
  let node = root;
  let path = '';
  for (const segment of segments) {
    path += `/${segment}`;
    let child = node.children.find((candidate) => candidate.label === segment);
    if (child === undefined) {
      child = { label: segment, path, entry: null, children: [], maxLevelValue: -1 };
      node.children.push(child);
    }
    node = child;
  }
  return node;
}

function computeMaxLevel(node: DiagnosticTreeNode): number {
  let worst = node.entry !== null ? node.entry.levelValue : -1;
  for (const child of node.children) {
    worst = Math.max(worst, computeMaxLevel(child));
  }
  node.maxLevelValue = worst;
  return worst;
}

function sortChildren(node: DiagnosticTreeNode): void {
  node.children.sort((a, b) => {
    if (b.maxLevelValue !== a.maxLevelValue) return b.maxLevelValue - a.maxLevelValue;
    return a.label.localeCompare(b.label);
  });
  for (const child of node.children) sortChildren(child);
}

/** Build a hierarchy of group nodes from aggregated diagnostics names. */
export function buildDiagnosticTree(entries: Iterable<DiagnosticEntry>): DiagnosticTreeNode {
  const root: DiagnosticTreeNode = { label: '', path: '', entry: null, children: [], maxLevelValue: -1 };
  for (const entry of entries) {
    const segments = normalizeName(entry.name);
    if (segments.length === 0) continue;
    ensureNode(root, segments).entry = entry;
  }
  computeMaxLevel(root);
  sortChildren(root);
  return root;
}

function findNode(node: DiagnosticTreeNode, path: string): DiagnosticTreeNode | null {
  if (node.path === path) return node;
  for (const child of node.children) {
    const found = findNode(child, path);
    if (found !== null) return found;
  }
  return null;
}

export function findDiagnosticNode(
  root: DiagnosticTreeNode,
  path: string | null,
): DiagnosticTreeNode | null {
  if (path === null) return null;
  return findNode(root, path);
}

/** Collect leaf entry names below a node (the node's own entry excluded unless it is a leaf). */
export function collectLeafNames(node: DiagnosticTreeNode): string[] {
  if (node.children.length === 0) return node.entry !== null ? [node.path] : [];
  return node.children.flatMap((child) => collectLeafNames(child));
}

/** Aggregate history points into per-second buckets of worst levels (-1 = no data). */
export function aggregateHistory(
  history: Record<string, DiagnosticPoint[]>,
  names: string[],
  now: number,
  windowMs: number,
  bucketCount: number,
): number[] {
  const buckets = new Array<number>(bucketCount).fill(-1);
  const start = now - windowMs;
  for (const name of names) {
    for (const point of history[name] ?? []) {
      if (point.t < start) continue;
      const index = Math.min(
        bucketCount - 1,
        Math.floor(((point.t - start) / windowMs) * bucketCount),
      );
      buckets[index] = Math.max(buckets[index] ?? -1, point.levelValue);
    }
  }
  return buckets;
}
