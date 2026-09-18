import type { Qos } from './qos.ts';

export interface TopicInfo {
  name: string;
  type: string;
}

export interface TopicEndpoint {
  info: TopicInfo;
  qos_profile: Qos;
}
