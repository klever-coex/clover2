import type { TopicInfo } from '../../types/topic.ts';
import type { HttpCall } from '../core.ts';

export interface TopicsEndpoints {
  list(): Promise<TopicInfo[]>;
}

export function createTopicsEndpoints(http: HttpCall): TopicsEndpoints {
  return {
    async list() {
      const response = await http<{ topics: TopicInfo[] }>('/api/topics', {
        capabilities: ['topics'],
      });
      return response.topics;
    },
  };
}
