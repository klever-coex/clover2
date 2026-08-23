import { clover2Api } from '../api/clover2.ts';
import type { NodeInfo } from '../types/node.ts';
import type { ServiceEndpoint } from '../types/service.ts';
import type { TopicEndpoint } from '../types/topic.ts';
import { useAsyncResource } from './useAsyncResource.ts';
import type { AsyncResource } from './useAsyncResource.ts';

export interface NodeResources {
  info: AsyncResource<NodeInfo>;
  publishers: AsyncResource<TopicEndpoint[]>;
  subscribes: AsyncResource<TopicEndpoint[]>;
  servers: AsyncResource<ServiceEndpoint[]>;
  clients: AsyncResource<ServiceEndpoint[]>;
}

export function useNodeResources(nodeName: string | null): NodeResources {
  const enabled = nodeName !== null;
  const name = nodeName ?? '';

  return {
    info: useAsyncResource(
      () => clover2Api.nodes.info(name),
      [name],
      enabled,
    ),
    publishers: useAsyncResource(
      () => clover2Api.nodes.publishers(name),
      [name],
      enabled,
    ),
    subscribes: useAsyncResource(
      () => clover2Api.nodes.subscribes(name),
      [name],
      enabled,
    ),
    servers: useAsyncResource(
      () => clover2Api.services.servers(name),
      [name],
      enabled,
    ),
    clients: useAsyncResource(
      () => clover2Api.services.clients(name),
      [name],
      enabled,
    ),
  };
}
