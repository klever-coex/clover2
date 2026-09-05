import type { TranslationKey } from '@/i18n/index.ts';

export interface NavRoute {
  path: string;
  labelKey: TranslationKey;
}

export const NAV_ROUTES: NavRoute[] = [
  { path: '/', labelKey: 'sidebar.dashboard' },
  { path: '/map', labelKey: 'sidebar.map' },
  { path: '/video', labelKey: 'sidebar.video' },
  { path: '/settings', labelKey: 'sidebar.settings' },
  { path: '/ros2/nodes', labelKey: 'sidebar.nodes' },
  { path: '/ros2/topics', labelKey: 'sidebar.topics' },
  { path: '/ros2/logs', labelKey: 'sidebar.logs' },
];

export function route(path: string): NavRoute {
  const found = NAV_ROUTES.find((r) => r.path === path);
  if (!found) throw new Error(`Unknown route: ${path}`);
  return found;
}

export function routeLabelKey(pathname: string): TranslationKey | undefined {
  const exact = NAV_ROUTES.find((r) => r.path === pathname);
  if (exact) return exact.labelKey;
  const prefixed = NAV_ROUTES.filter((r) => pathname.startsWith(`${r.path}/`)).sort(
    (a, b) => b.path.length - a.path.length,
  )[0];
  return prefixed?.labelKey;
}
