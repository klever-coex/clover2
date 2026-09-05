import { ChevronDown } from 'lucide-react';

import type { SettingsScalar, SettingsSchemaNode } from '@/types/settings';
import { isScalarNode } from '@/types/settings';
import { Badge } from '@/components/ui/badge';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Collapsible, CollapsibleContent, CollapsibleTrigger } from '@/components/ui/collapsible';
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
    <Card>
      <Collapsible defaultOpen className="group/collapsible w-full">
        <CardHeader>
          <CollapsibleTrigger className="flex w-full items-center justify-between gap-2 text-left">
            <span className="flex min-w-0 items-center gap-2">
              <CardTitle className="font-mono">{node.name}</CardTitle>
              <Badge variant="secondary">{scalars.length}</Badge>
            </span>
            <ChevronDown className="size-4 shrink-0 text-muted-foreground transition-transform duration-fast group-data-[state=open]/collapsible:rotate-180" />
          </CollapsibleTrigger>
        </CardHeader>
        <CollapsibleContent>
          <CardContent>
            <div className="flex flex-col gap-2">
              {node.description !== undefined && (
                <p className="text-xs text-muted-foreground">{node.description}</p>
              )}

              <div className="divide-y divide-border">
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
          </CardContent>
        </CollapsibleContent>
      </Collapsible>
    </Card>
  );
}
