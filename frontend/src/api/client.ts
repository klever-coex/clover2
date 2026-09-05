import type { Clover2Api } from './clover2.ts';
import { createHttpCall } from './core.ts';
import { createManifestEndpoints } from './endpoints/manifest.ts';
import { createMapEndpoints } from './endpoints/map.ts';
import { createNodesEndpoints } from './endpoints/nodes.ts';
import { createServicesEndpoints } from './endpoints/services.ts';
import { createSettingsEndpoints } from './endpoints/settings.ts';
import { createStreamsEndpoints } from './endpoints/streams.ts';
import { createTopicsEndpoints } from './endpoints/topics.ts';
import { baseUrlMiddleware } from './middleware/baseUrl.ts';
import { capabilityMiddleware } from './middleware/capability.ts';
import { errorsMiddleware } from './middleware/errors.ts';
import { jsonMiddleware } from './middleware/json.ts';
import { createFetchExecutor } from './transport.ts';
import { resolveBaseUrl, toWebSocketBase } from './url.ts';

export function createClient(baseUrl?: string): Clover2Api {
  const httpBase = baseUrl ?? resolveBaseUrl();
  const wsBase = toWebSocketBase(httpBase);

  const executor = createFetchExecutor();

  const openHttp = createHttpCall(
    httpBase,
    [baseUrlMiddleware, errorsMiddleware, jsonMiddleware],
    executor,
  );

  const manifestEndpoints = createManifestEndpoints(openHttp);

  const http = createHttpCall(
    httpBase,
    [
      baseUrlMiddleware,
      capabilityMiddleware(manifestEndpoints.requireCapability),
      errorsMiddleware,
      jsonMiddleware,
    ],
    executor,
  );

  const streams = createStreamsEndpoints(
    wsBase,
    manifestEndpoints.requireCapability,
  );

  return {
    manifest: {
      get: manifestEndpoints.get,
      clearCache: manifestEndpoints.clearCache,
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
