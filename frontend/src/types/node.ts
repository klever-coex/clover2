export type LifecycleState = 'unconfigured' | 'inactive' | 'active' | 'finalized';

export interface NodeInfo {
  name: string;
  ns: string;
  is_lifecycle: boolean;
  lifecycle_state?: LifecycleState;
}
