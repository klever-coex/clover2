import { useState } from 'react';
import { RotateCcw } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import type { SettingsScalar, SettingsSchemaNode } from '../../types/settings.ts';
import { IconButton } from '../ui/IconButton.tsx';
import { Select } from '../ui/Select.tsx';
import { TypeBadge } from '../ros2/TypeBadge.tsx';
import { Badge } from '../ui/Badge.tsx';

interface Props {
  node: SettingsSchemaNode;
  path: string[];
  onValue: (path: string[], value: SettingsScalar) => void;
  onReset: (path: string[]) => void;
}

const inputClasses =
  'w-56 rounded-row border border-line bg-surface-1 px-2 py-1.5 text-xs font-mono text-ink outline-none transition-colors duration-fast focus:border-accent/60 focus:ring-2 focus:ring-accent/20';

export function SettingsFieldRow({ node, path, onValue, onReset }: Props) {
  const { t } = useTranslation();
  const isDefault = node.value !== undefined && node.value === node.default;

  return (
    <div className="flex items-start justify-between gap-4 py-2.5">
      <div className="min-w-0">
        <div className="flex items-center gap-2">
          <span className="font-mono text-sm text-ink break-all">{node.name}</span>
          <TypeBadge type={node.type} />
          {isDefault && <Badge tone="neutral">{t('settings.defaultValue')}</Badge>}
        </div>
        {node.description !== undefined && (
          <p className="mt-0.5 text-xs text-ink-muted">{node.description}</p>
        )}
      </div>

      <div className="flex items-center gap-1.5 shrink-0">
        <FieldControl node={node} path={path} onValue={onValue} />
        {node.value !== node.default && node.default !== undefined && (
          <IconButton
            icon={RotateCcw}
            size="sm"
            label={t('settings.resetField')}
            onClick={() => onReset(path)}
          />
        )}
      </div>
    </div>
  );
}

function FieldControl({
  node,
  path,
  onValue,
}: Pick<Props, 'node' | 'path' | 'onValue'>) {
  const { t } = useTranslation();
  const [local, setLocal] = useState(() => formatValue(node.value));
  const [invalid, setInvalid] = useState<string | null>(null);

  const [prevCommitted, setPrevCommitted] = useState(node.value);
  if (node.value !== prevCommitted) {
    setPrevCommitted(node.value);
    setLocal(formatValue(node.value));
    setInvalid(null);
  }

  const commit = () => {
    if (node.type === 'int') {
      if (!/^[+-]?\d+$/.test(local.trim())) {
        setInvalid(t('settings.invalidInt'));
        return;
      }
      onValue(path, Number(local));
    } else if (node.type === 'float') {
      const parsed = Number(local);
      if (local.trim() === '' || !Number.isFinite(parsed)) {
        setInvalid(t('settings.invalidFloat'));
        return;
      }
      onValue(path, parsed);
    } else {
      onValue(path, local);
    }
    setInvalid(null);
  };

  if (node.type === 'bool') {
    return (
      <input
        type="checkbox"
        checked={node.value === true}
        onChange={(e) => onValue(path, e.target.checked)}
        className="size-4 accent-[var(--color-accent)]"
      />
    );
  }

  if (node.enum !== undefined) {
    return (
      <Select
        value={String(node.value ?? '')}
        onChange={(value) => onValue(path, value)}
        options={node.enum.map((option) => ({ value: option, label: option }))}
        className="w-40"
      />
    );
  }

  return (
    <div className="flex flex-col items-end">
      <input
        type="text"
        value={local}
        onChange={(e) => setLocal(e.target.value)}
        onKeyDown={(e) => {
          if (e.key === 'Enter') (e.target as HTMLInputElement).blur();
        }}
        onBlur={commit}
        className={inputClasses}
      />
      {invalid !== null && <span className="mt-0.5 text-[10px] text-error">{invalid}</span>}
    </div>
  );
}

function formatValue(value: SettingsScalar | undefined): string {
  if (value === undefined || value === null) return '';
  return String(value);
}
