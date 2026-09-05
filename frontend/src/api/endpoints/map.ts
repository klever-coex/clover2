import { MUTATION_TIMEOUT_MS } from '../../constants/ros.ts';
import type { MapInfo, MarkerInfo, ModifyResult } from '@/types/map';
import type { HttpCall } from '../core.ts';

export interface MapEndpoints {
  get(): Promise<MapInfo>;
  marker(id: number): Promise<MarkerInfo>;
  add(marker: MarkerInfo): Promise<ModifyResult>;
  edit(id: number, marker: MarkerInfo): Promise<ModifyResult>;
  delete(id: number): Promise<ModifyResult>;
}

export function createMapEndpoints(http: HttpCall): MapEndpoints {
  return {
    get: () => http<MapInfo>('/api/map', { capabilities: ['map'] }),

    marker: (id) =>
      http<MarkerInfo>(`/api/map/marker/-/${id}`, { capabilities: ['map'] }),

    add: (marker) =>
      http<ModifyResult>('/api/map/marker', {
        capabilities: ['map'],
        method: 'POST',
        body: marker,
        timeoutMs: MUTATION_TIMEOUT_MS,
      }),

    edit: (id, marker) =>
      http<ModifyResult>(`/api/map/marker/-/${id}`, {
        capabilities: ['map'],
        method: 'PUT',
        body: marker,
        timeoutMs: MUTATION_TIMEOUT_MS,
      }),

    delete: (id) =>
      http<ModifyResult>(`/api/map/marker/-/${id}`, {
        capabilities: ['map'],
        method: 'DELETE',
        timeoutMs: MUTATION_TIMEOUT_MS,
      }),
  };
}
