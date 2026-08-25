import { useRef, useMemo, useLayoutEffect } from 'react';
import * as THREE from 'three';
import { TransformControls } from '@react-three/drei';
import { useThree } from '@react-three/fiber';

import { DEFAULT_GRID_STEP_MM } from '../../../constants/defaults.ts';
import { mmToM } from '../../../constants/units.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import { useMapUIStore } from '../../../store/useMapUIStore.ts';
import { applyPose } from '../../../pages/map/mutations.ts';
import type { Vec3 } from '../../../types/marker.ts';
import { sanitizeVec3, vec3ToPositionExpr } from '../../../utils/transformUtils.ts';

/**
 * Finds the selected marker's object. three.js `getObjectByProperty` compares by
 * strict object identity while R3F replaces userData on every render, so a fresh
 * literal can never match — traverse and compare the markerId field instead.
 */
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
  // eslint-disable-next-line @typescript-eslint/no-explicit-any -- drei ref type is opaque
  const controlsRef = useRef<any>(null);
  const gridStepM = useMemo(() => mmToM(DEFAULT_GRID_STEP_MM), []);

  // Remove the 2D plane handles and the free-drag center — keep only the axis
  // arrows. three-stdlib's gizmo re-asserts `handle.visible = true` on every
  // rendered frame (TransformControlsGizmo.updateMatrixWorld), so hiding via
  // `.visible` can never work. The gizmo group is built once per controls
  // instance — removing the meshes once on mount is durable and idempotent.
  useLayoutEffect(() => {
    const controls = controlsRef.current as any | null;
    if (controls === null) return;

    const gizmo: THREE.Object3D | null =
      (controls.gizmo as THREE.Object3D | undefined) ?? (() => {
        let found: THREE.Object3D | null = null;
        (controls as THREE.Object3D).traverse((obj) => {
          if (found === null && (obj as { isTransformControlsGizmo?: boolean }).isTransformControlsGizmo) {
            found = obj;
          }
        });
        return found;
      })();
    if (gizmo === null) return;

    // Visible handles live in gizmo.gizmo['translate']; invisible raycast
    // targets in gizmo.picker['translate'] — remove the planes from both so
    // plane-drag picking is disabled too.
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

  // Find the Three.js object for the selected marker
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
          void applyPose(sel);
        }
        setTransforming(false);
        dragPosRef.current = null;
      }}
    />
  );
}
