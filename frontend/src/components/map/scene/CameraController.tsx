import { useRef } from 'react';
import { OrbitControls } from '@react-three/drei';
import { useThree } from '@react-three/fiber';

export function CameraController() {
  const controlsRef = useRef<any>(null);
  const { invalidate } = useThree();

  return (
    <OrbitControls
      ref={controlsRef}
      makeDefault
      enableDamping
      dampingFactor={0.1}
      minPolarAngle={0}
      maxPolarAngle={Math.PI * 0.85}
      minDistance={0.1}
      maxDistance={50}
      zoomSpeed={1.2}
      panSpeed={0.8}
      onChange={() => invalidate()}
    />
  );
}
