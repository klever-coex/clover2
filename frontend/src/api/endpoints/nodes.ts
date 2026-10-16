import type { LifecycleTransitionDescription } from '@/types/lifecycle';
import type { NodeInfo } from '@/types/node';
import type { TopicEndpoint } from '@/types/topic';
import { MUTATION_TIMEOUT_MS } from '../../constants/ros.ts';
import type { HttpCall } from '../core.ts';
import { encodeRosPath } from '../url.ts';

export interface NodesEndpoints {
  list(): Promise<string[]>;
  info(nodeName: string): Promise<NodeInfo>;
  publishers(nodeName: string): Promise<TopicEndpoint[]>;
  subscribes(nodeName: string): Promise<TopicEndpoint[]>;
  availableTransitions(nodeName: string): Promise<
    LifecycleTransitionDescription[]
  >;
  transition(nodeName: string, transition: string): Promise<void>;
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

    async availableTransitions(nodeName) {
      const response = await http<{
        available_transitions: LifecycleTransitionDescription[];
      }>(`/api/node/lifecycle/available_transitions/-/${encodeRosPath(nodeName)}`, {
        capabilities: ['nodes'],
      });
      return response.available_transitions;
    },

    async transition(nodeName, transition) {
      await http<void>(
        `/api/node/lifecycle/transition/-/${encodeRosPath(nodeName)}`,
        {
          capabilities: ['nodes'],
          method: 'POST',
          body: { label: transition },
          // Transitions run user callbacks (on_activate etc.) and can easily
          // outlive the default request timeout.
          timeoutMs: MUTATION_TIMEOUT_MS,
        },
      );
    },
  };
}
