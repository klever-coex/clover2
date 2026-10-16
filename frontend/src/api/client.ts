import type { Clover2Api } from './clover2.ts';
import { createHttpCall } from './core.ts';
import { createManifestEndpoints } from './endpoints/manifest.ts';
import { createMapEndpoints } from './endpoints/map.ts';
import { createNodesEndpoints } from './endpoints/nodes.ts';
import { createServicesEndpoints } from './endpoints/services.ts';
import { createSettingsEndpoints } from './endpoints/settings.ts';
import { createStreamsEndpoints } from './endpoints/streams.ts';
import { createTopicsEndpoints } from './endpoints/topics.ts';
import { resolveBaseUrl, toWebSocketBase } from './url.ts';

export function createClient(baseUrl?: string): Clover2Api {
  const httpBase = baseUrl ?? resolveBaseUrl();
  const wsBase = toWebSocketBase(httpBase);

  const openHttp = createHttpCall(httpBase);

  const manifestEndpoints = createManifestEndpoints(openHttp);

  const http = createHttpCall(httpBase, manifestEndpoints.requireCapability);

  const streams = createStreamsEndpoints(
    wsBase,
    manifestEndpoints.requireCapability,
  );

  return {
    manifest: {
      get: manifestEndpoints.get,
    },
    topics: {
      ...createTopicsEndpoints(http),
      subscribe: streams.subscribe,
    },
    nodes: createNodesEndpoints(http),
    services: createServicesEndpoints(http),
    map: createMapEndpoints(http),
    settings: createSettingsEndpoints(http),
  };
}
