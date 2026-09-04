import { useRef, useMemo, useLayoutEffect } from 'react';
import * as THREE from 'three';
import { TransformControls } from '@react-three/drei';
import type { TransformControls as TransformControlsImpl } from 'three-stdlib';
import { useThree } from '@react-three/fiber';

import { DEFAULT_GRID_STEP_MM } from '../../../constants/defaults.ts';
import { mmToM } from '../../../constants/units.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import { useMapUIStore } from '../../../store/useMapUIStore.ts';
import type { Vec3 } from '../../../types/marker.ts';
import { sanitizeVec3, vec3ToPositionExpr } from '../../../utils/transformUtils.ts';

function findMarkerObject(scene: THREE.Scene, id: string): THREE.Object3D | null {
  let found: THREE.Object3D | null = null;
  scene.traverse((obj) => {
    if (found === null && (obj.userData as { markerId?: string } | undefined)?.markerId === id) {
      found = obj;
    }
  });
  return found;
}

export function TransformController() {
  const { scene, invalidate } = useThree();
  const selectedIds = useMapStore((s) => s.selectedMarkerIds);
  const markers = useMapStore((s) => s.markers);
  const setExpr = useMapStore((s) => s.setExpr);
  const setTransforming = useMapUIStore((s) => s.setTransforming);

  const dragPosRef = useRef<Vec3 | null>(null);
  const controlsRef = useRef<TransformControlsImpl | null>(null);
  const gridStepM = useMemo(() => mmToM(DEFAULT_GRID_STEP_MM), []);

  useLayoutEffect(() => {
    const controls = controlsRef.current;
    if (controls === null) return;

    const gizmo: THREE.Object3D | null =
      (controls as unknown as { gizmo?: THREE.Object3D }).gizmo ??
      (() => {
        let found: THREE.Object3D | null = null;
        (controls as unknown as THREE.Object3D).traverse((obj) => {
          if (found === null && (obj as { isTransformControlsGizmo?: boolean }).isTransformControlsGizmo) {
            found = obj;
          }
        });
        return found;
      })();

    if (gizmo === null) return;

    for (const groupKey of ['gizmo', 'picker'] as const) {
      const group = (gizmo as unknown as Record<string, Record<string, THREE.Group>>)[groupKey]?.['translate'];
      if (group === undefined) continue;
      for (const name of ['XY', 'YZ', 'XZ', 'XYZ']) {
        const handle = group.children.find((child) => child.name === name);
        if (handle !== undefined) group.remove(handle);
      }
    }
  }, []);

  const sel = selectedIds.length === 1 ? (selectedIds[0] ?? null) : null;
  const marker = sel !== null ? markers[sel] : undefined;
  const isVisible = marker !== undefined && marker.type === 'fixed' && marker.pose !== null;

  const targetObj = useMemo(() => {
    if (!isVisible || sel === null) return null;
    return findMarkerObject(scene, sel);
  }, [scene, isVisible, sel]);

  if (!isVisible || sel === null || targetObj === null) return null;

  return (
    <TransformControls
      ref={controlsRef}
      object={targetObj}
      mode="translate"
      translationSnap={gridStepM}
      showX={isVisible}
      showY={isVisible}
      showZ={isVisible}
      onObjectChange={() => {
        const pos = targetObj.position;
        dragPosRef.current = sanitizeVec3([pos.x, pos.y, pos.z]);
        invalidate();
      }}
      onPointerDown={() => {
        setTransforming(true);
      }}
      onPointerUp={() => {
        if (dragPosRef.current) {
          const expr = vec3ToPositionExpr(dragPosRef.current);
          for (const axis of [0, 1, 2] as const) {
            setExpr(sel, axis, 'position', expr[axis]);
          }
        }
        setTransforming(false);
        dragPosRef.current = null;
      }}
    />
  );
}
