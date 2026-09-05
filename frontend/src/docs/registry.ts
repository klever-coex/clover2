import { docsBaseUrl } from '../constants/docs.ts';

export type DocKey = 'ros2.node.lifecycle' | 'programming.settings';

interface DocEntry {
  article: string;
  anchor?: string;
}

const DOC_ENTRIES: Record<DocKey, DocEntry> = {
  'ros2.node.lifecycle': {
    article: 'UsefulInformation/ROS2/Nodes',
    anchor: 'lifecycle-node',
  },
  'programming.settings': {
    article: 'Programming/DroneSettings',
  },
};

export function docsUrl(key: DocKey, _lang?: string): string {
  const entry = DOC_ENTRIES[key];
  const anchor = entry.anchor !== undefined ? `#${entry.anchor}` : '';
  // TODO: add en documentation
  return `${docsBaseUrl("ru")}${entry.article}.html${anchor}`;
}
