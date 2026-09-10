import type { StateCreator } from 'zustand';

import { toApiError, type ApiError } from '@/types/errors';

export type ResourceSlice<Name extends string, T> = { [K in Name]: T } & {
  [K in `${Name}Loading`]: boolean;
} & { [K in `${Name}Error`]: ApiError | null } & {
  [K in `reload${Capitalize<Name>}`]: () => Promise<void>;
};

export interface ResourceConfig<Name extends string, T> {
  name: Name;
  initial: T;
  fetcher: () => Promise<T>;
}

export function createResourceSlice<S, Name extends string, T>(
  config: ResourceConfig<Name, T>,
): StateCreator<S, [], [], ResourceSlice<Name, T>> {
  return (set, get) => {
    const loadingKey = `${config.name}Loading` as const;
    const errorKey = `${config.name}Error` as const;

    const slice = {
      [config.name]: config.initial,
      [loadingKey]: false,
      [errorKey]: null,
      [`reload${config.name[0]!.toUpperCase()}${config.name.slice(1)}`]: async () => {
        const current = get() as unknown as ResourceSlice<Name, T>;
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
    } as ResourceSlice<Name, T>;

    return slice;
  };
}
