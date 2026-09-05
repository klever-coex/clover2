import type { LifecycleState } from './node.ts';

export type LifecycleStateLabel = LifecycleState | 'unknown' | (string & {});

export type TransitionState =
  | 'configuring'
  | 'cleaningup'
  | 'shuttingdown'
  | 'activating'
  | 'deactivating'
  | 'errorprocessing';

export interface LabeledState {
  label: LifecycleStateLabel;
}

export interface LabeledTransition {
  label: string;
}

export interface LifecycleTransitionDescription {
  transition: LabeledTransition;
  start_state: LabeledState;
  goal_state: LabeledState;
}
