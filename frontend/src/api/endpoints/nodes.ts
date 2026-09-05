import type { NodeInfo } from '@/types/node';
import type { TopicEndpoint } from '@/types/topic';
import type { HttpCall } from '../core.ts';
import { encodeRosPath } from '../url.ts';

export interface NodesEndpoints {
  list(): Promise<string[]>;
  info(nodeName: string): Promise<NodeInfo>;
  publishers(nodeName: string): Promise<TopicEndpoint[]>;
  subscribes(nodeName: string): Promise<TopicEndpoint[]>;
}

export function createNodesEndpoints(http: HttpCall): NodesEndpoints {
  return {
    async list() {
      const response = await http<{ nodes: string[] }>('/api/nodes', {
        capabilities: ['nodes'],
      });
      return response.nodes;
    },

    info: (nodeName) =>
      http<NodeInfo>(`/api/node/info/-/${encodeRosPath(nodeName)}`, {
        capabilities: ['nodes'],
      }),

    async publishers(nodeName) {
      const response = await http<{ topics: TopicEndpoint[] }>(
        `/api/node/publishers/-/${encodeRosPath(nodeName)}`,
        { capabilities: ['nodes'] },
      );
      return response.topics;
    },

    async subscribes(nodeName) {
      const response = await http<{ topics: TopicEndpoint[] }>(
        `/api/node/subscribes/-/${encodeRosPath(nodeName)}`,
        { capabilities: ['nodes'] },
      );
      return response.topics;
    },
  };
}
