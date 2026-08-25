import { clover2Api } from '../api/clover2.ts';
import type { NodeInfo } from '../types/node.ts';
import { useAsyncResource } from './useAsyncResource.ts';
import type { AsyncResource } from './useAsyncResource.ts';

/**
 * Fetches info for all nodes in one batch (allSettled): a failed request just
 * leaves that node without info — the map only holds fulfilled results.
 */
export function useNodeInfos(names: readonly string[]): AsyncResource<Map<string, NodeInfo>> {
  return useAsyncResource(
    async () => {
      const results = await Promise.allSettled(
        names.map((name) => clover2Api.nodes.info(name)),
      );
      const infos = new Map<string, NodeInfo>();
      results.forEach((result, index) => {
        if (result.status === 'fulfilled') {
          infos.set(names[index]!, result.value);
        }
      });
      return infos;
    },
    [names],
  );
}
