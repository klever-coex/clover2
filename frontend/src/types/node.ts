import type { ServiceEndpoint } from './service.ts';
import type { TopicEndpoint } from './topic.ts';

export interface NodeInfo {
  name: string;
  ns: string;
  is_lifecycle: boolean;
}

export interface NodeTopics {
  topics: TopicEndpoint[];
}

export interface NodeServices {
  services: ServiceEndpoint[];
}
