export function SceneLighting() {
  return (
    <>
      <ambientLight intensity={0.3} />
      <directionalLight
        position={[5, 3, 10]}
        intensity={1.5}
        castShadow
        shadow-mapSize-width={1024}
        shadow-mapSize-height={1024}
        shadow-camera-far={50}
        shadow-camera-left={-10}
        shadow-camera-right={10}
        shadow-camera-top={10}
        shadow-camera-bottom={-10}
        shadow-bias={-0.0001}
      />
      <hemisphereLight args={['#87CEEB', '#3a5f3a', 0.4]} />
    </>
  );
}
