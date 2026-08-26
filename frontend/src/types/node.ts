import type { ServiceEndpoint } from './service.ts';
import type { TopicEndpoint } from './topic.ts';

export type LifecycleState = 'unconfigured' | 'inactive' | 'active' | 'finalized';

export interface NodeInfo {
  name: string;
  ns: string;
  is_lifecycle: boolean;
  lifecycle_state?: LifecycleState;
}

export interface NodeTopics {
  topics: TopicEndpoint[];
}

export interface NodeServices {
  services: ServiceEndpoint[];
}
