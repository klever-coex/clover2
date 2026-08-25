import { Grid } from '@react-three/drei';

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
      cellColor="#444444"
      sectionColor="#666666"
      renderOrder={0}
    />
  );
}
