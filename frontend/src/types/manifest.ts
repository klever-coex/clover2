export type Capability = 'nodes' | 'topics' | 'services' | (string & {});

export interface PluginManifest {
  name: string;
  version: number;
  capabilities: Capability[];
}

export interface Manifest {
  framework_version: string;
  plugins: PluginManifest[];
}
