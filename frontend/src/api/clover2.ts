import type { Manifest } from '@/types/manifest';
import type { MapInfo, MarkerInfo, ModifyResult } from '@/types/map';
import type { NodeInfo } from '@/types/node';
import type { ServiceEndpoint } from '@/types/service';
import type { SettingsModifyResult, SettingsSchemaResponse } from '@/types/settings';
import type { TopicEndpoint, TopicInfo } from '@/types/topic';

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

export interface MapApi {
  get(): Promise<MapInfo>;
  marker(id: number): Promise<MarkerInfo>;
  add(marker: MarkerInfo): Promise<ModifyResult>;
  edit(id: number, marker: MarkerInfo): Promise<ModifyResult>;
  delete(id: number): Promise<ModifyResult>;
}

export interface SettingsApi {
  schema(): Promise<SettingsSchemaResponse>;
  save(values: unknown): Promise<SettingsModifyResult>;
}

export interface Clover2Api {
  readonly manifest: ManifestApi;
  readonly topics: TopicsApi;
  readonly nodes: NodesApi;
  readonly services: ServicesApi;
  readonly map: MapApi;
  readonly settings: SettingsApi;
}

export function createClover2Api(baseUrl?: string): Clover2Api {
  return createClient(baseUrl);
}

export const clover2Api: Clover2Api = createClover2Api();
