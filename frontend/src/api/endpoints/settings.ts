import { MUTATION_TIMEOUT_MS } from '../../constants/ros.ts';
import type { SettingsModifyResult, SettingsSchemaResponse } from '../../types/settings.ts';
import type { HttpCall } from '../core.ts';

export interface SettingsEndpoints {
  schema(): Promise<SettingsSchemaResponse>;
  save(values: unknown): Promise<SettingsModifyResult>;
}

export function createSettingsEndpoints(http: HttpCall): SettingsEndpoints {
  return {
    schema: () =>
      http<SettingsSchemaResponse>('/api/settings/schema', {
        capabilities: ['settings'],
      }),

    save: (values) =>
      http<SettingsModifyResult>('/api/settings/values', {
        capabilities: ['settings'],
        method: 'PUT',
        body: { values },
        timeoutMs: MUTATION_TIMEOUT_MS,
      }),
  };
}
