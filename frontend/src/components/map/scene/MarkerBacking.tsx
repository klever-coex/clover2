import { PLACEHOLDER_GRAY } from '@/constants/scene';

interface Props {
  sizeM: number;
}

export function MarkerBacking({ sizeM }: Props) {
  const thickness = Math.max(sizeM * 0.01, 0.001);

  return (
    <mesh
      position={[0, 0, -thickness / 2]}
      raycast={() => {}}
      renderOrder={0}
    >
      <boxGeometry args={[sizeM, sizeM, thickness]} />
      <meshStandardMaterial
        color={PLACEHOLDER_GRAY}
        depthWrite={true}
      />
    </mesh>
  );
}
