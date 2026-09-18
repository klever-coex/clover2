import { useRosStore } from '@/store/useRosStore';
import { cn } from '@/lib/utils';

interface SidebarVersionProps {
  className?: string;
}

export function SidebarVersion({ className }: SidebarVersionProps) {
  const frameworkVersion = useRosStore((s) => s.manifest?.framework_version);

  const lines = [
    __CLOVER2_VERSION__ !== '' ? `frontend v${__CLOVER2_VERSION__}` : null,
    frameworkVersion !== undefined ? `framework v${frameworkVersion}` : null,
  ].filter((line) => line !== null);

  if (lines.length === 0) return null;

  return (
    <div
      className={cn(
        'font-mono text-micro text-muted-foreground/60 group-data-[collapsible=icon]:hidden',
        className,
      )}
    >
      {lines.map((line) => (
        <div key={line}>{line}</div>
      ))}
    </div>
  );
}
