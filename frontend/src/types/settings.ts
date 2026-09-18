export type SettingsFieldType = 'str' | 'bool' | 'int' | 'float' | 'object';

export type SettingsScalar = boolean | number | string | null;

export interface SettingsSchemaNode {
  name: string;
  type: SettingsFieldType;
  description?: string;
  enum?: string[];
  default?: SettingsScalar;
  value?: SettingsScalar;
  children?: SettingsSchemaNode[];
}

export interface SettingsSchemaResponse {
  valid: boolean;
  root: SettingsSchemaNode;
}

export interface SettingsModifyResult {
  success: boolean;
  error_message: string;
}

export type SettingsValuesTree = { [field: string]: SettingsScalar | SettingsValuesTree };

export function isScalarNode(node: SettingsSchemaNode): boolean {
  return node.type !== 'object';
}

export function toValuesTree(node: SettingsSchemaNode): SettingsValuesTree {
  const out: SettingsValuesTree = {};

  for (const child of node.children ?? []) {
    if (isScalarNode(child)) {
      out[child.name] = child.value ?? null;
    } else {
      out[child.name] = toValuesTree(child);
    }
  }

  return out;
}

export function updateLeaf(
  node: SettingsSchemaNode,
  path: readonly string[],
  update: (leaf: SettingsSchemaNode) => SettingsSchemaNode,
): SettingsSchemaNode {
  if (path.length === 0) return update(node);
  if (node.type !== 'object') return node;

  const [head, ...rest] = path;
  let changed = false;
  const children = (node.children ?? []).map((child) => {
    if (child.name !== head) return child;
    const updated = updateLeaf(child, rest, update);
    changed = updated !== child;
    return updated;
  });

  return changed ? { ...node, children } : node;
}

export function valuesProjection(node: SettingsSchemaNode): string {
  if (isScalarNode(node)) return JSON.stringify(node.value ?? null);

  const parts = (node.children ?? []).map(
    (child) => `${JSON.stringify(child.name)}:${valuesProjection(child)}`,
  );
  return `{${parts.join(',')}}`;
}

