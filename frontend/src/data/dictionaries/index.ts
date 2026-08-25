import type { ArUcoDictionary, DictionaryData, DictionaryMeta } from '../../types/marker.ts';

export type { DictionaryData, DictionaryMeta };

const DICT_META: Record<ArUcoDictionary, DictionaryMeta> = {
  'DICT_4X4_1000': { name: 'DICT_4X4_1000', gridSize: 4, maxMarkerId: 999, totalCells: 6 },
  'DICT_5X5_1000': { name: 'DICT_5X5_1000', gridSize: 5, maxMarkerId: 999, totalCells: 7 },
  'DICT_6X6_1000': { name: 'DICT_6X6_1000', gridSize: 6, maxMarkerId: 999, totalCells: 8 },
  'DICT_7X7_1000': { name: 'DICT_7X7_1000', gridSize: 7, maxMarkerId: 999, totalCells: 9 },
  'DICT_ARUCO_ORIGINAL':  { name: 'DICT_ARUCO_ORIGINAL',  gridSize: 5, maxMarkerId: 1023, totalCells: 7 },
  'DICT_APRILTAG_16h5':  { name: 'DICT_APRILTAG_16h5',  gridSize: 4, maxMarkerId: 29,   totalCells: 6 },
  'DICT_APRILTAG_25h9':  { name: 'DICT_APRILTAG_25h9',  gridSize: 5, maxMarkerId: 34,   totalCells: 7 },
  'DICT_APRILTAG_36h10': { name: 'DICT_APRILTAG_36h10', gridSize: 6, maxMarkerId: 2319, totalCells: 8 },
  'DICT_APRILTAG_36h11': { name: 'DICT_APRILTAG_36h11', gridSize: 6, maxMarkerId: 586,  totalCells: 8 },
};

const DICT_BASE: Record<ArUcoDictionary, string> = {
  'DICT_4X4_1000': 'DICT_4X4_1000',
  'DICT_5X5_1000': 'DICT_5X5_1000',
  'DICT_6X6_1000': 'DICT_6X6_1000',
  'DICT_7X7_1000': 'DICT_7X7_1000',
  'DICT_ARUCO_ORIGINAL':  'DICT_ARUCO_ORIGINAL',
  'DICT_APRILTAG_16h5':  'DICT_APRILTAG_16h5',
  'DICT_APRILTAG_25h9':  'DICT_APRILTAG_25h9',
  'DICT_APRILTAG_36h10': 'DICT_APRILTAG_36h10',
  'DICT_APRILTAG_36h11': 'DICT_APRILTAG_36h11',
};

interface MarkerJsonFile {
  dictionary: string;
  markerSize: number;
  markers: number[][];
}

const promiseCache = new Map<ArUcoDictionary, Promise<DictionaryData>>();
const fetchCache = new Map<string, Promise<MarkerJsonFile>>();

export function loadDictionary(name: ArUcoDictionary): Promise<DictionaryData> {
  let promise = promiseCache.get(name);
  if (!promise) {
    promise = loadAndParse(name);
    promiseCache.set(name, promise);
  }
  return promise;
}

async function fetchWithRetry(baseName: string, retries = 2): Promise<MarkerJsonFile> {
  for (let i = 0; i <= retries; i++) {
    try {
      const resp = await fetch(`${import.meta.env.BASE_URL}markers/${baseName}.json`);
      if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
      return await resp.json() as MarkerJsonFile;
    } catch (e) {
      if (i === retries) throw e;
      await new Promise(r => setTimeout(r, 1000 * (i + 1)));
    }
  }
  throw new Error('unreachable');
}

async function fetchJson(baseName: string): Promise<MarkerJsonFile> {
  if (!fetchCache.has(baseName)) {
    fetchCache.set(baseName, fetchWithRetry(baseName));
  }
  return fetchCache.get(baseName)!;
}

async function loadAndParse(name: ArUcoDictionary): Promise<DictionaryData> {
  const meta = DICT_META[name];
  if (!meta) throw new Error(`Unknown dictionary: ${name}`);

  const baseName = DICT_BASE[name];
  const json = await fetchJson(baseName);

  const count = meta.maxMarkerId + 1;
  const rawMarkers = json.markers.slice(0, count);
  const bytesList = rawMarkers.map((bytes) => new Uint8Array(bytes));

  return { meta, bytesList };
}

export function getDictionaryMeta(name: ArUcoDictionary): DictionaryMeta {
  const meta = DICT_META[name];
  if (!meta) throw new Error(`Unknown dictionary: ${name}`);
  return meta;
}

export function isDictionaryName(name: string): name is ArUcoDictionary {
  return name in DICT_META;
}

export { DICT_META };
