import { Grid } from '@react-three/drei';

import { SCENE_COLORS } from '@/constants/scene';

export function GroundGrid() {
  return (
    <Grid
      position={[0, 0, 0]}
      rotation={[Math.PI / 2, 0, 0]}
      infiniteGrid
      cellSize={1}
      cellThickness={0.5}
      sectionSize={5}
      sectionThickness={1}
      fadeDistance={30}
      cellColor={SCENE_COLORS.gridCell}
      sectionColor={SCENE_COLORS.gridSection}
      renderOrder={0}
    />
  );
}
