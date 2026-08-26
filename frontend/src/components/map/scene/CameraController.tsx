import { useRef, useEffect } from 'react';
import { OrbitControls } from '@react-three/drei';
import { useThree } from '@react-three/fiber';

import { useMapUIStore } from '../../../store/useMapUIStore.ts';

export function CameraController() {
  // drei does not export a ref type for the controls instance.
  const controlsRef = useRef<any>(null);
  const isTransforming = useMapUIStore((s) => s.isTransforming);
  const { invalidate } = useThree();

  useEffect(() => {
    if (controlsRef.current) {
      controlsRef.current.enabled = !isTransforming;
    }
  }, [isTransforming]);

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
