import type { ServiceEndpoint } from '@/types/service';
import type { HttpCall } from '../core.ts';
import { encodeRosPath } from '../url.ts';

export interface ServicesEndpoints {
  servers(nodeName: string): Promise<ServiceEndpoint[]>;
  clients(nodeName: string): Promise<ServiceEndpoint[]>;
}

export function createServicesEndpoints(http: HttpCall): ServicesEndpoints {
  return {
    async servers(nodeName) {
      const response = await http<{ services: ServiceEndpoint[] }>(
        `/api/node/servers/-/${encodeRosPath(nodeName)}`,
        { capabilities: ['services'] },
      );
      return response.services;
    },

    async clients(nodeName) {
      const response = await http<{ services: ServiceEndpoint[] }>(
        `/api/node/clients/-/${encodeRosPath(nodeName)}`,
        { capabilities: ['services'] },
      );
      return response.services;
    },
  };
}
