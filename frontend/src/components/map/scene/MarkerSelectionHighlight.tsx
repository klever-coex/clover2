import * as THREE from 'three';

interface Props {
  sizeM: number;
}

export function MarkerSelectionHighlight({ sizeM }: Props) {
  const highlightSize = sizeM + 0.01;

  return (
    <mesh position={[0, 0, 0.001]} renderOrder={3}>
      <planeGeometry args={[highlightSize, highlightSize]} />
      <meshBasicMaterial
        color="#ffaa00"
        side={THREE.DoubleSide}
        transparent
        opacity={0.25}
        depthTest={true}
        depthWrite={false}
      />
    </mesh>
  );
}
