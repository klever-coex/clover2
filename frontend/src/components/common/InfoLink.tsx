import { useTranslation } from 'react-i18next';
import { Info } from 'lucide-react';

import { cn } from '@/lib/utils';
import { docsUrl } from '@/docs/registry';
import type { DocKey } from '@/docs/registry';
import { Tooltip, TooltipContent, TooltipTrigger } from '@/components/ui/tooltip';

interface InfoLinkProps {
  docKey: DocKey;
  className?: string;
}

/** Small "info" icon linking to a specific paragraph of the documentation. */
export function InfoLink({ docKey, className }: InfoLinkProps) {
  const { t, i18n } = useTranslation();
  const label = t('common.moreInDocs');

  return (
    <Tooltip>
      <TooltipTrigger asChild>
        <a
          href={docsUrl(docKey, i18n.language)}
          target="_blank"
          rel="noreferrer"
          aria-label={label}
          className={cn(
            'inline-flex size-6 shrink-0 items-center justify-center rounded-row text-muted-foreground transition-colors duration-fast hover:bg-muted hover:text-foreground focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring/60',
            className,
          )}
        >
          <Info className="size-4" />
        </a>
      </TooltipTrigger>
      <TooltipContent>{label}</TooltipContent>
    </Tooltip>
  );
}
