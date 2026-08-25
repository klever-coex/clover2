import { useRef, useMemo } from 'react';
import * as THREE from 'three';
import { Line, Html } from '@react-three/drei';

interface Props {
  /** Total length of each axis arm in meters (default 1) */
  length?: number;
  /** Position of the axes origin in world space */
  position?: [number, number, number];
  /** Show axis labels (X, Y, Z) at tips */
  showLabels?: boolean;
  /** Opacity of the axis lines (0–1, default 1) */
  opacity?: number;
}

const COLORS = {
  x: '#ff4444', // red
  y: '#44ff44', // green
  z: '#4444ff', // blue
};

const AXES: { axis: 'x' | 'y' | 'z'; dir: [number, number, number]; color: string }[] = [
  { axis: 'x', dir: [1, 0, 0], color: COLORS.x },
  { axis: 'y', dir: [0, 1, 0], color: COLORS.y },
  { axis: 'z', dir: [0, 0, 1], color: COLORS.z },
];

export function AxesHelper({ length = 1, position = [0, 0, 0], showLabels = true, opacity = 1 }: Props) {
  const groupRef = useRef<THREE.Group>(null);

  const points = useMemo(() => {
    const result: Record<string, [number, number, number][]> = {};
    for (const { axis, dir } of AXES) {
      const neg: [number, number, number] = [0.0, 0.0, 0.0];
      const pos: [number, number, number] = [
        dir[0] * length,
        dir[1] * length,
        dir[2] * length,
      ];
      result[axis] = [neg, pos];
    }
    return result;
  }, [length]);

  const lineStyle = useMemo(
    () => ({
      lineWidth: 2,
      transparent: true,
      opacity,
      depthTest: true,
      depthWrite: false,
    }),
    [opacity],
  );

  const labelPositions = useMemo(() => {
    const offset = length + 0.05;
    return AXES.map(({ axis, dir }) => ({
      axis,
      pos: [dir[0] * offset, dir[1] * offset, dir[2] * offset] as [number, number, number],
    }));
  }, [length]);

  return (
    <group ref={groupRef} position={position} renderOrder={999}>
      {AXES.map(({ axis, color }) => (
        <Line
          key={axis}
          points={points[axis]!}
          color={color}
          {...lineStyle}
        />
      ))}
      {showLabels &&
        labelPositions.map(({ axis, pos }) => (
          <Html key={`label-${axis}`} position={pos} center style={{ pointerEvents: 'none' }}>
            <span
              style={{
                color: COLORS[axis],
                fontSize: 12,
                fontWeight: 700,
                fontFamily: 'monospace',
                textShadow: '0 0 6px rgba(0,0,0,0.8)',
              }}
            >
              {axis.toUpperCase()}
            </span>
          </Html>
        ))}
    </group>
  );
}
