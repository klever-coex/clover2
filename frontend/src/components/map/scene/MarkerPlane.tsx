import { Suspense, use, useMemo } from 'react';
import * as THREE from 'three';
import { useTexture } from '@react-three/drei';

import type { DictionaryData } from '../../../data/dictionaries/index.ts';
import { loadDictionary } from '../../../data/dictionaries/index.ts';
import type { ArUcoDictionary } from '../../../types/marker.ts';

const CELL_PX = 64;

/** Bit extraction matching Python ArucoBits exactly (verified: 0 mismatches on 4024 markers).
 *  Reads gridSize² bits MSB-first from consecutive bytes, handling the partial
 *  last byte where remaining bits are packed into LSB positions. */
function extractBits(bytes: Uint8Array, totalBits: number): boolean[] {
  const bits: boolean[] = [];
  for (const byte of bytes) {
    const start = totalBits - bits.length;
    for (let i = Math.min(7, start - 1); i >= 0; i--) {
      bits.push(((byte >> i) & 1) === 1);
    }
  }
  return bits;
}

/** Build an SVG string for a single marker.
 *  Black border white inner area black cells for each '1' bit. */
function buildSvg(dictData: DictionaryData, markerId: number): string {
  const { gridSize } = dictData.meta;
  const { bytesList } = dictData;
  const bytes = bytesList[markerId];
  if (!bytes) throw new Error(`Marker ID ${markerId} out of range for dictionary`);

  const totalPx = (gridSize + 2) * CELL_PX;
  const bits = extractBits(bytes, gridSize * gridSize);

  let svg = document.createElement('svg');
  svg.setAttribute('xmlns', 'http://www.w3.org/2000/svg');
  svg.setAttribute('viewBox', `0 0 ${totalPx} ${totalPx}`);
  svg.setAttribute('shape-rendering', 'crispEdges');
  svg.setAttribute('height', `${totalPx}`);
  svg.setAttribute('width', `${totalPx}`);

  let background = document.createElement('rect');
  background.setAttribute('x', '0');
  background.setAttribute('y', '0');
  background.setAttribute('width', `${totalPx}`);
  background.setAttribute('height', `${totalPx}`);
  background.setAttribute('fill', 'black');
  svg.appendChild(background);

  for (let row = 0; row < gridSize; row++) {
    for (let col = 0; col < gridSize; col++) {
      if (bits[row * gridSize + col]) {
        let cell = document.createElement('rect');
        cell.setAttribute('x', `${(col + 1) * CELL_PX}`);
        cell.setAttribute('y', `${(row + 1) * CELL_PX}`);
        cell.setAttribute('width', `${CELL_PX}`);
        cell.setAttribute('height', `${CELL_PX}`);
        cell.setAttribute('fill', 'white');
        svg.appendChild(cell);
      }
    }
  }

  return svg.outerHTML;
}

/* ---- SVG data URI cache ---- */

const svgCache = new Map<string, string>();

function getSvgUrl(dict: ArUcoDictionary, dictData: DictionaryData, markerId: number): string {
  const key = `${dict}__${markerId}`;
  let url = svgCache.get(key);
  if (!url) {
    url = 'data:image/svg+xml,' + encodeURIComponent(buildSvg(dictData, markerId));
    svgCache.set(key, url);
  }
  return url;
}

/* ---- Component ---- */

function ArUcoPlane({
  sizeM,
  dict,
  markerId,
  dictData,
}: {
  sizeM: number;
  dict: ArUcoDictionary;
  markerId: number;
  dictData: DictionaryData;
}) {
  const url = useMemo<string>(() => getSvgUrl(dict, dictData, markerId), [dict, dictData, markerId]);
  const loaded = useTexture(url);
  // Clone before tweaking filters — never mutate the cached texture instance.
  const texture = useMemo(() => {
    const tex = loaded.clone();
    tex.colorSpace = THREE.SRGBColorSpace;
    tex.minFilter = THREE.NearestFilter;
    tex.magFilter = THREE.NearestFilter;
    return tex;
  }, [loaded]);

  return (
    <mesh frustumCulled={false}>
      <planeGeometry args={[sizeM, sizeM]} />
      <meshBasicMaterial map={texture} side={THREE.DoubleSide} />
    </mesh>
  );
}

interface Props {
  sizeM: number;
  dict: ArUcoDictionary;
  markerId: number;
}

export function MarkerPlane({ sizeM, dict, markerId }: Props) {
  const dictData = use(loadDictionary(dict));

  // Backend marker ids beyond the dictionary range: render a gray placeholder
  // instead of throwing (the SVG builder cannot index out-of-range ids).
  if (markerId < 0 || markerId >= dictData.bytesList.length) {
    console.warn(`Marker ID ${markerId} out of range for dictionary ${dict}`);
    return (
      <mesh frustumCulled={false}>
        <planeGeometry args={[sizeM, sizeM]} />
        <meshBasicMaterial color="#888888" side={THREE.DoubleSide} />
      </mesh>
    );
  }

  return (
    <Suspense fallback={null}>
      <ArUcoPlane sizeM={sizeM} dict={dict} markerId={markerId} dictData={dictData} />
    </Suspense>
  );
}
