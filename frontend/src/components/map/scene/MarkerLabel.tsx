import { Html } from '@react-three/drei';

import { useMapUIStore } from '@/store/useMapUIStore';

interface Props {
  text: string;
  yOffset: number;
}

export function MarkerLabel({ text, yOffset }: Props) {
  const modalOpen = useMapUIStore((s) => s.modalOpen);
  if (modalOpen) return null;

  return (
    <Html position={[0, 0, yOffset]} center zIndexRange={[40, 0]} style={{ pointerEvents: 'none' }}>
      <span className="rounded px-1.5 py-0.5 font-mono text-[11px] whitespace-nowrap bg-foreground/70 text-background">
        {text}
      </span>
    </Html>
  );
}
