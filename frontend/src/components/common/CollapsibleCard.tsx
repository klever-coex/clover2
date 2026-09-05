import type { ReactNode } from 'react';
import { ChevronDown } from 'lucide-react';

import { Badge } from '@/components/ui/badge';
import { Card, CardContent, CardHeader } from '@/components/ui/card';
import {
  Collapsible,
  CollapsibleContent,
  CollapsibleTrigger,
} from '@/components/ui/collapsible';

interface CollapsibleCardProps {
  title: ReactNode;
  count?: number;
  defaultOpen?: boolean;
  children: ReactNode;
}

export function CollapsibleCard({
  title,
  count,
  defaultOpen,
  children,
}: CollapsibleCardProps) {
  return (
    <Card>
      <Collapsible defaultOpen={defaultOpen} className="group/collapsible-card w-full">
        <CardHeader>
          {/* asChild: the trigger renders a <button>, and buttons must not
              contain the block-level <div> that CardTitle renders. */}
          <CollapsibleTrigger asChild>
            <button className="flex w-full cursor-pointer items-center gap-2 text-left">
              <span className="flex min-w-0 items-center gap-2 text-base leading-snug font-medium">
                {title}
                {count !== undefined && <Badge variant="secondary">{count}</Badge>}
              </span>
              <ChevronDown className="ml-auto size-4 shrink-0 text-muted-foreground transition-transform duration-fast group-data-[state=open]/collapsible-card:rotate-180" />
            </button>
          </CollapsibleTrigger>
        </CardHeader>
        <CollapsibleContent className="overflow-hidden data-[state=closed]:animate-collapsible-up data-[state=open]:animate-collapsible-down">
          <CardContent>{children}</CardContent>
        </CollapsibleContent>
      </Collapsible>
    </Card>
  );
}
