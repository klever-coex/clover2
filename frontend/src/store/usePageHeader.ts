import { useEffect, useLayoutEffect } from 'react';
import type { ReactNode } from 'react';
import { create } from 'zustand';

export interface PageCrumb {
  label: string;
  to?: string;
  mono?: boolean;
}

interface PageHeaderState {
  crumbs: PageCrumb[];
  actions: ReactNode | null;
  set: (crumbs: PageCrumb[], actions: ReactNode | null) => void;
  clear: () => void;
}

let lastCrumbsJson = '';
let lastActions: ReactNode | null = null;

export const usePageHeaderStore = create<PageHeaderState>((set) => ({
  crumbs: [],
  actions: null,
  set: (crumbs, actions) => {
    const signature = JSON.stringify(crumbs);
    if (signature === lastCrumbsJson && actions === lastActions) return;
    lastCrumbsJson = signature;
    lastActions = actions;
    set({ crumbs, actions });
  },
  clear: () => {
    if (lastCrumbsJson === '[]' && lastActions === null) return;
    lastCrumbsJson = '[]';
    lastActions = null;
    set({ crumbs: [], actions: null });
  },
}));

export function usePageHeader(crumbs: PageCrumb[], actions?: ReactNode) {
  const set = usePageHeaderStore((s) => s.set);
  const clear = usePageHeaderStore((s) => s.clear);

  useLayoutEffect(() => {
    set(crumbs, actions ?? null);
    return () => clear();
  });

  useEffect(() => {
    const label = crumbs[crumbs.length - 1]?.label;
    document.title = label !== undefined ? `${label} · Clover2` : 'Clover2';
  }, [crumbs]);
}
