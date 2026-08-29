import type { SettingsScalar, SettingsSchemaNode } from '../../types/settings.ts';
import { isScalarNode } from '../../types/settings.ts';
import { Panel } from '../ui/Panel.tsx';
import { SettingsFieldRow } from './SettingsFieldRow.tsx';

interface Props {
  node: SettingsSchemaNode;
  path: string[];
  onValue: (path: string[], value: SettingsScalar) => void;
  onReset: (path: string[]) => void;
}

export function SettingsSection({ node, path, onValue, onReset }: Props) {
  const scalars = (node.children ?? []).filter(isScalarNode);
  const objects = (node.children ?? []).filter((child) => !isScalarNode(child));

  return (
    <Panel title={<span className="font-mono">{node.name}</span>} count={scalars.length} collapsible defaultOpen>
      <div className="space-y-2">
        {node.description !== undefined && (
          <p className="text-xs text-ink-muted">{node.description}</p>
        )}

        <div className="divide-y divide-border-soft">
          {scalars.map((child) => (
            <SettingsFieldRow
              key={child.name}
              node={child}
              path={[...path, child.name]}
              onValue={onValue}
              onReset={onReset}
            />
          ))}
        </div>

        {objects.map((child) => (
          <SettingsSection
            key={child.name}
            node={child}
            path={[...path, child.name]}
            onValue={onValue}
            onReset={onReset}
          />
        ))}
      </div>
    </Panel>
  );
}
