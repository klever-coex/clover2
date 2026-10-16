import { useMemo } from 'react';
import { Line } from '@react-three/drei';

import { SCENE_COLORS } from '@/constants/scene';

const AXES = [
  { dir: [1, 0, 0] as const, color: SCENE_COLORS.axisX },
  { dir: [0, 1, 0] as const, color: SCENE_COLORS.axisY },
  { dir: [0, 0, 1] as const, color: SCENE_COLORS.axisZ },
];

export function AxesHelper({ length = 1 }: { length?: number }) {
  const lines = useMemo(
    () =>
      AXES.map(({ dir, color }) => ({
        color,
        points: [
          [0, 0, 0],
          [dir[0] * length, dir[1] * length, dir[2] * length],
        ] as [number, number, number][],
      })),
    [length],
  );

  return (
    <group renderOrder={999}>
      {lines.map(({ color, points }) => (
        <Line key={color} points={points} color={color} lineWidth={2} depthWrite={false} />
      ))}
    </group>
  );
}
