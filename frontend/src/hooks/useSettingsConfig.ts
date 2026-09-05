import { useCallback, useState } from 'react';

import { clover2Api } from '../api/clover2.ts';
import { toApiError } from '@/types/errors';
import type { ApiError } from '@/types/errors';
import type { SettingsScalar, SettingsSchemaNode, SettingsSchemaResponse } from '@/types/settings';
import { toValuesTree, updateLeaf, valuesProjection } from '@/types/settings';
import { useAsyncResource } from './useAsyncResource.ts';

export interface SettingsConfig {
  loading: boolean;
  error: ApiError | null;
  root: SettingsSchemaNode | null;
  dirty: boolean;
  saving: boolean;
  saveError: string | null;
  setValue: (path: readonly string[], value: SettingsScalar) => void;
  resetField: (path: readonly string[]) => void;
  resetAll: () => void;
  save: () => Promise<void>;
  reload: () => Promise<void>;
}

export function useSettingsConfig(): SettingsConfig {
  const resource = useAsyncResource(() => clover2Api.settings.schema(), []);

  const [adopted, setAdopted] = useState<SettingsSchemaResponse | null>(null);
  const [edited, setEdited] = useState<SettingsSchemaNode | null>(null);
  const [saving, setSaving] = useState(false);
  const [saveError, setSaveError] = useState<string | null>(null);

  if (resource.data !== adopted) {
    setAdopted(resource.data);
    setEdited(resource.data?.root ?? null);
    setSaveError(null);
  }

  const root = edited;
  const snapshot = resource.data !== null ? valuesProjection(resource.data.root) : '';
  const dirty = root !== null && valuesProjection(root) !== snapshot;

  const setValue = useCallback((path: readonly string[], value: SettingsScalar) => {
    setSaveError(null);
    setEdited((current) =>
      current === null ? current : updateLeaf(current, path, (leaf) => ({ ...leaf, value })),
    );
  }, []);

  const resetField = useCallback((path: readonly string[]) => {
    setSaveError(null);
    setEdited((current) =>
      current === null
        ? current
        : updateLeaf(current, path, (leaf) => ({ ...leaf, value: leaf.default })),
    );
  }, []);

  const resetAll = useCallback(() => {
    setSaveError(null);
    setEdited((current) => {
      if (current === null) return current;
      const resetTree = (node: SettingsSchemaNode): SettingsSchemaNode =>
        node.type === 'object'
          ? { ...node, children: (node.children ?? []).map(resetTree) }
          : { ...node, value: node.default };
      return resetTree(current);
    });
  }, []);

  const save = useCallback(async () => {
    if (root === null || saving) return;
    setSaving(true);
    setSaveError(null);
    try {
      const result = await clover2Api.settings.save(toValuesTree(root));
      if (!result.success) {
        setSaveError(result.error_message);
        return;
      }
      await resource.reload();
    } catch (err) {
      setSaveError(toApiError(err).message);
    } finally {
      setSaving(false);
    }
  }, [resource, root, saving]);

  return {
    loading: resource.loading,
    error: resource.error,
    root,
    dirty,
    saving,
    saveError,
    setValue,
    resetField,
    resetAll,
    save,
    reload: async () => resource.reload(),
  };
}
