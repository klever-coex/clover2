import { Html } from '@react-three/drei';

interface Props {
  text: string;
  yOffset: number;
}

export function MarkerLabel({ text, yOffset }: Props) {
  return (
    <Html position={[0, 0, yOffset]} center style={{ pointerEvents: 'none' }}>
      <span style={{
        background: 'rgba(0, 0, 0, 0.7)',
        color: '#fff',
        padding: '2px 6px',
        borderRadius: 4,
        fontSize: 11,
        fontFamily: 'monospace',
        whiteSpace: 'nowrap',
      }}>
        {text}
      </span>
    </Html>
  );
}
