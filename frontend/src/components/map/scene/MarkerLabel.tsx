import { Html } from '@react-three/drei';

interface Props {
  text: string;
  yOffset: number;
}

export function MarkerLabel({ text, yOffset }: Props) {
  return (
    <Html position={[0, 0, yOffset]} center style={{ pointerEvents: 'none' }}>
      <span className="rounded px-1.5 py-0.5 font-mono text-[11px] whitespace-nowrap bg-foreground/70 text-background">
        {text}
      </span>
    </Html>
  );
}
