import type { StateCreator } from 'zustand';

import { toApiError } from '../../types/errors.ts';
import type { ApiError } from '../../types/errors.ts';

export type ResourceSlice<
  Name extends string,
  T,
  FetchAction extends string = `reload${Capitalize<Name>}`,
> = { [K in Name]: T } & { [K in `${Name}Loading`]: boolean } & {
  [K in `${Name}Error`]: ApiError | null;
} & { [K in FetchAction]: () => Promise<void> };

export interface ResourceConfig<
  Name extends string,
  T,
  FetchAction extends string = `reload${Capitalize<Name>}`,
> {
  name: Name;
  initial: T;
  fetcher: () => Promise<T>;
  fetch?: FetchAction;
}

function capitalize(value: string): string {
  return value.charAt(0).toUpperCase() + value.slice(1);
}

export function createResourceSlice<
  S,
  Name extends string,
  T,
  FetchAction extends string = `reload${Capitalize<Name>}`,
>(
  config: ResourceConfig<Name, T, FetchAction>,
): StateCreator<S, [], [], ResourceSlice<Name, T, FetchAction>> {
  return (set, get) => {
    const loadingKey = `${config.name}Loading` as const;
    const errorKey = `${config.name}Error` as const;
    const fetchAction = (config.fetch ??
      `reload${capitalize(config.name)}`) as FetchAction;

    const slice = {
      [config.name]: config.initial,
      [loadingKey]: false,
      [errorKey]: null,
      [fetchAction]: async () => {
        const current = get() as unknown as ResourceSlice<Name, T, FetchAction>;
        if (current[loadingKey]) return;

        set({ [loadingKey]: true, [errorKey]: null } as unknown as Partial<S>);

        try {
          const data = await config.fetcher();
          set({ [config.name]: data, [loadingKey]: false } as unknown as Partial<S>);
        } catch (error) {
          set({
            [loadingKey]: false,
            [errorKey]: toApiError(error),
          } as unknown as Partial<S>);
        }
      },
    } as ResourceSlice<Name, T, FetchAction>;

    return slice;
  };
}
