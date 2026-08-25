import { memo, useRef, useCallback, useMemo } from 'react';
import * as THREE from 'three';
import type { ThreeEvent } from '@react-three/fiber';

import { DEFAULT_DICTIONARY } from '../../../constants/defaults.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import { resolvePosition, resolveRotation } from '../../../utils/transformUtils.ts';
import { MarkerBacking } from './MarkerBacking.tsx';
import { MarkerLabel } from './MarkerLabel.tsx';
import { MarkerPlane } from './MarkerPlane.tsx';
import { MarkerSelectionHighlight } from './MarkerSelectionHighlight.tsx';

interface Props {
  markerId: string;
}

export const MarkerMesh = memo(function MarkerMesh({ markerId }: Props) {
  const marker = useMapStore((s) => s.markers[markerId]);
  const ui = useMapStore((s) => s.markerUi[markerId]);
  const isSelected = useMapStore((s) => s.selectedMarkerIds.includes(markerId));
  const selectMarker = useMapStore((s) => s.selectMarker);
  const dictionary = useMapStore((s) => s.mapMeta?.dictionary ?? DEFAULT_DICTIONARY);

  const groupRef = useRef<THREE.Group>(null);

  // Compute position from expression at render time
  const positionM = useMemo(() => {
    if (!marker || !ui) return [0, 0, 0] as const;
    return resolvePosition(ui.positionExpr, marker.id);
  }, [marker, ui]);

  // Compute rotation from expression at render time
  const rotationEuler = useMemo(() => {
    if (!marker || !ui) return new THREE.Euler();
    const q = resolveRotation(ui.rotationExpr, marker.id);
    const quat = new THREE.Quaternion(q[0], q[1], q[2], q[3]);
    return new THREE.Euler().setFromQuaternion(quat, 'XYZ');
  }, [marker, ui]);

  const handleClick = useCallback(
    (e: ThreeEvent<MouseEvent>) => {
      e.stopPropagation();
      if (marker) selectMarker(markerId, e.metaKey || e.ctrlKey);
    },
    [marker, markerId, selectMarker],
  );

  // Only fixed markers with a known pose are rendered in 3D.
  if (!marker || !ui || marker.type !== 'fixed' || marker.pose === null) return null;

  return (
    <group
      ref={groupRef}
      position={positionM}
      rotation={rotationEuler}
      visible={ui.visible}
      onClick={handleClick}
      userData={{ markerId }}
    >
      <group position={[0, 0, 0.005]}>
        {isSelected && <MarkerSelectionHighlight sizeM={marker.sizeM} />}
        <MarkerPlane sizeM={marker.sizeM} dict={dictionary} markerId={marker.id} />
      </group>
      <MarkerBacking sizeM={marker.sizeM} />
      <MarkerLabel text={marker.markerFrameId || `#${marker.id}`} yOffset={marker.sizeM / 2 + 0.02} />
    </group>
  );
});
