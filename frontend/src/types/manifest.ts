// Wire types of the clover2_http backend (default port 8080).

export type Capability = 'nodes' | 'topics' | 'services' | (string & {});

export interface PluginManifest {
  name: string;
  version: number;
  capabilities: Capability[];
}

export interface Manifest {
  plugins: PluginManifest[];
}
