import { create } from 'zustand';

export interface ConfirmOptions {
  message: string;
  title?: string;
  tone?: 'default' | 'danger';
  confirmLabel?: string;
  cancelLabel?: string;
}

interface ConfirmState {
  options: ConfirmOptions | null;
  show: (options: ConfirmOptions) => Promise<boolean>;
  settle: (result: boolean) => void;
}

let resolver: ((result: boolean) => void) | null = null;

export const useConfirmStore = create<ConfirmState>((set) => ({
  options: null,
  show: (options) =>
    new Promise<boolean>((resolve) => {
      resolver?.(false);
      resolver = resolve;
      set({ options });
    }),
  settle: (result) => {
    resolver?.(result);
    resolver = null;
    set({ options: null });
  },
}));

export function confirmDialog(options: ConfirmOptions): Promise<boolean> {
  return useConfirmStore.getState().show(options);
}
