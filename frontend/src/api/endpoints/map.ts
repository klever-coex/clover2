import type { MapInfo, MarkerInfo, ModifyResult } from '../../types/map.ts';
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
      }),

    edit: (id, marker) =>
      http<ModifyResult>(`/api/map/marker/-/${id}`, {
        capabilities: ['map'],
        method: 'PUT',
        body: marker,
      }),

    delete: (id) =>
      http<ModifyResult>(`/api/map/marker/-/${id}`, {
        capabilities: ['map'],
        method: 'DELETE',
      }),
  };
}
