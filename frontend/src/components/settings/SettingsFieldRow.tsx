import { useState } from 'react';
import { RotateCcw } from 'lucide-react';
import { useTranslation } from 'react-i18next';

import type { SettingsScalar, SettingsSchemaNode } from '@/types/settings';
import { cn } from '@/lib/utils';
import { inputSm } from '@/lib/uiStyles';
import { Badge } from '@/components/ui/badge';
import { Switch } from '@/components/ui/switch';
import { Field, FieldContent, FieldError, FieldLabel } from '@/components/ui/field';
import { Input } from '@/components/ui/input';
import { Select, SelectContent, SelectGroup, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { TooltipButton } from '../common/TooltipButton.tsx';
import { TypeBadge } from '../common/TypeBadge.tsx';

interface Props {
  node: SettingsSchemaNode;
  path: string[];
  onValue: (path: string[], value: SettingsScalar) => void;
  onReset: (path: string[]) => void;
}

const inputClasses = cn(inputSm, 'w-full');

export function SettingsFieldRow({ node, path, onValue, onReset }: Props) {
  const { t } = useTranslation();
  const fieldId = `settings-${path.join('-')}`;
  const isDefault = node.value !== undefined && node.value === node.default;
  const showReset = !isDefault && node.default !== undefined;

  return (
    <Field orientation="horizontal" className="py-2.5">
      <FieldContent className="min-w-0">
        <FieldLabel htmlFor={fieldId}>
          <span className="font-mono text-sm break-all text-foreground">{node.name}</span>
          <TypeBadge type={node.type} />
          {isDefault && <Badge variant="secondary">{t('settings.defaultValue')}</Badge>}
        </FieldLabel>
        {node.description !== undefined && (
          <p className="mt-0.5 text-xs text-muted-foreground">{node.description}</p>
        )}
      </FieldContent>

      <div className="flex shrink-0 items-center gap-1.5">
        {node.default !== undefined && (
          <span className={cn('flex items-center', showReset ? 'visible' : 'invisible')}>
            <TooltipButton
              variant="ghost"
              size="icon-sm"
              label={t('settings.resetField')}
              tabIndex={showReset ? 0 : -1}
              onClick={() => onReset(path)}
            >
              <RotateCcw />
            </TooltipButton>
          </span>
        )}
        <FieldControl
          node={node}
          path={path}
          onValue={onValue}
          id={fieldId}
        />
      </div>
    </Field>
  );
}

interface FieldControlProps {
  node: SettingsSchemaNode;
  path: string[];
  onValue: (path: string[], value: SettingsScalar) => void;
  id: string;
}

function FieldControl({ node, path, onValue, id }: FieldControlProps) {
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
      <Switch
        checked={node.value === true}
        onCheckedChange={(checked) => onValue(path, checked)}
        aria-label={node.name}
        className="data-checked:bg-foreground"
      />
    );
  }

  if (node.enum !== undefined) {
    return (
      <Select
        value={String(node.value ?? '')}
        onValueChange={(value) => onValue(path, value)}
      >
        <SelectTrigger aria-label={node.name} size="sm" className="w-40 font-mono">
          <SelectValue />
        </SelectTrigger>
        <SelectContent>
          <SelectGroup>
            {node.enum.map((option) => (
              <SelectItem key={option} value={option}>
                {option}
              </SelectItem>
            ))}
          </SelectGroup>
        </SelectContent>
      </Select>
    );
  }

  return (
    <Field data-invalid={invalid !== null || undefined}>
      <div className="flex flex-col items-end gap-0.5">
        <Input
          id={id}
          type="text"
          aria-label={node.name}
          value={local}
          onChange={(e) => setLocal(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === 'Enter') (e.target as HTMLInputElement).blur();
          }}
          onBlur={commit}
          aria-invalid={invalid !== null}
          className={inputClasses}
        />
        {invalid !== null && <FieldError>{invalid}</FieldError>}
      </div>
    </Field>
  );
}

function formatValue(value: SettingsScalar | undefined): string {
  if (value === undefined || value === null) return '';
  return String(value);
}
