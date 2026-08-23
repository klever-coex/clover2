import { useEffect, useEffectEvent, useState } from 'react';
import type { DependencyList } from 'react';

import { toApiError } from '../types/errors.ts';
import type { ApiError } from '../types/errors.ts';

export interface AsyncResource<T> {
  data: T | null;
  loading: boolean;
  error: ApiError | null;
  reload: () => void;
}

export function useAsyncResource<T>(
  fetcher: () => Promise<T>,
  deps: DependencyList,
  enabled = true,
): AsyncResource<T> {
  // Always calls the latest fetcher without re-running the effect.
  const runFetch = useEffectEvent(fetcher);

  const [data, setData] = useState<T | null>(null);
  const [loading, setLoading] = useState(enabled);
  const [error, setError] = useState<ApiError | null>(null);
  const [generation, setGeneration] = useState(0);

  // Any dep / generation / enabled change starts a new request. Reset the
  // in-flight state during render so the UI never shows the previous
  // request's loading/error state.
  const depsKey = JSON.stringify(deps);
  const requestKey = `${enabled}:${generation}:${depsKey}`;
  const [prevKey, setPrevKey] = useState(requestKey);
  if (prevKey !== requestKey) {
    setPrevKey(requestKey);
    if (!enabled) {
      setData(null);
    }
    setLoading(enabled);
    setError(null);
  }

  useEffect(() => {
    if (!enabled) return;

    let cancelled = false;

    runFetch().then(
      (result) => {
        if (!cancelled) {
          setData(result);
          setLoading(false);
        }
      },
      (reason: unknown) => {
        if (!cancelled) {
          setError(toApiError(reason));
          setLoading(false);
        }
      },
    );

    return () => {
      cancelled = true;
    };
  }, [depsKey, generation, enabled]);

  return {
    data,
    loading,
    error,
    reload: () => setGeneration((current) => current + 1),
  };
}
