import { Canvas, useThree } from '@react-three/fiber';
import { Suspense, useEffect } from 'react';

import { DEFAULT_DICTIONARY } from '../../../constants/defaults.ts';
import { loadDictionary } from '../../../data/dictionaries/index.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import { useMapUIStore } from '../../../store/useMapUIStore.ts';
import { AxesHelper } from './AxesHelper.tsx';
import { CameraController } from './CameraController.tsx';
import { GroundGrid } from './GroundGrid.tsx';
import { MarkerMesh } from './MarkerMesh.tsx';
import { SceneCanvasContextHandler } from './SceneCanvasContextHandler.tsx';
import { SceneLighting } from './SceneLighting.tsx';
import { TransformController } from './TransformController.tsx';

// Preload the default dictionary so markers render without a blank-scene flash
void loadDictionary(DEFAULT_DICTIONARY);

/** Invalidates the R3F render loop when markers change (since we use frameloop="demand") */
function StoreInvalidator() {
  const invalidate = useThree((s) => s.invalidate);
  const markers = useMapStore((s) => s.markers);
  const markerUi = useMapStore((s) => s.markerUi);
  const selectedIds = useMapStore((s) => s.selectedMarkerIds);

  useEffect(() => {
    invalidate();
  }, [markers, markerUi, selectedIds, invalidate]);

  return null;
}

export function SceneCanvas() {
  const markers = useMapStore((s) => s.markers);
  const mapMeta = useMapStore((s) => s.mapMeta);
  const deselectAll = useMapStore((s) => s.deselectAll);
  const showAxes = useMapUIStore((s) => s.showAxes);

  // Preload the map's dictionary once it is known (usually == DEFAULT_DICTIONARY).
  useEffect(() => {
    if (mapMeta !== null) {
      void loadDictionary(mapMeta.dictionary);
    }
  }, [mapMeta]);

  return (
    <Canvas
      frameloop="demand"
      dpr={[1, 1.5]}
      camera={{ position: [2, -5, 3], up: [0, 0, 1], fov: 50, near: 0.01, far: 100 }}
      shadows={true}
      gl={{
        outputColorSpace: 'srgb',
        toneMapping: 0, // NoToneMapping — was 4 (ACES) which crushes colors
        preserveDrawingBuffer: false,
        antialias: true,
      }}
      onPointerMissed={() => deselectAll()}
      style={{ position: 'absolute', top: 0, left: 0, width: '100%', height: '100%' }}
    >
      <color attach="background" args={['#0F172A']} />
      <StoreInvalidator />
      <SceneCanvasContextHandler />
      <Suspense fallback={null}>
        <SceneLighting />
        <GroundGrid />
        <CameraController />
        {Object.keys(markers).map((id) => (
          <MarkerMesh key={id} markerId={id} />
        ))}
        <TransformController />
        {showAxes && <AxesHelper length={0.5} />}
      </Suspense>
    </Canvas>
  );
}
