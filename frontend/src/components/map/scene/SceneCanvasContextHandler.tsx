import { useThree } from '@react-three/fiber';
import { useEffect } from 'react';

export function SceneCanvasContextHandler() {
  const { gl, invalidate } = useThree();

  useEffect(() => {
    const handleLost = (e: Event) => {
      e.preventDefault();
      console.warn('WebGL context lost');
    };

    const handleRestored = () => {
      // SVG textures via useTexture auto-recover on context restore —
      // Three.js re-uploads from the in-memory Image (data URI).
      invalidate();
    };

    const canvas = gl.domElement;
    canvas.addEventListener('webglcontextlost', handleLost);
    canvas.addEventListener('webglcontextrestored', handleRestored);

    return () => {
      canvas.removeEventListener('webglcontextlost', handleLost);
      canvas.removeEventListener('webglcontextrestored', handleRestored);
    };
  }, [gl, invalidate]);

  return null;
}
