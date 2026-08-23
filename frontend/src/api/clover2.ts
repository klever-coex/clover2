import type { Manifest } from '../types/manifest.ts';
import type { NodeInfo } from '../types/node.ts';
import type { ServiceEndpoint } from '../types/service.ts';
import type { TopicEndpoint, TopicInfo } from '../types/topic.ts';

import { createClient } from './client.ts';
import type {
  TopicSubscription,
  TopicSubscriptionOptions,
} from './endpoints/streams.ts';

export type { TopicSubscription, TopicSubscriptionOptions } from './endpoints/streams.ts';

export interface ManifestApi {
  get(): Promise<Manifest>;
  clearCache(): void;
}

export interface TopicsApi {
  list(): Promise<TopicInfo[]>;
  subscribe(
    topicName: string,
    options: TopicSubscriptionOptions,
  ): TopicSubscription;
}

export interface NodesApi {
  list(): Promise<string[]>;
  info(nodeName: string): Promise<NodeInfo>;
  publishers(nodeName: string): Promise<TopicEndpoint[]>;
  subscribes(nodeName: string): Promise<TopicEndpoint[]>;
}

export interface ServicesApi {
  servers(nodeName: string): Promise<ServiceEndpoint[]>;
  clients(nodeName: string): Promise<ServiceEndpoint[]>;
}

export interface Clover2Api {
  readonly manifest: ManifestApi;
  readonly topics: TopicsApi;
  readonly nodes: NodesApi;
  readonly services: ServicesApi;
}

export function createClover2Api(baseUrl?: string): Clover2Api {
  return createClient(baseUrl);
}

export const clover2Api: Clover2Api = createClover2Api();
