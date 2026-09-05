export interface Qos {
  /** 'keep_last' | 'keep_all' | 'system_default' | 'unknown' */
  history: string;
  depth: number;
  /** 'reliable' | 'best_effort' | 'best_available' | 'system_default' | 'unknown' */
  reliability: string;
  /** 'volatile' | 'transient_local' | 'best_available' | 'system_default' | 'unknown' */
  durability: string;
  /** 'automatic' | 'manual_by_topic' | 'best_available' | 'system_default' | 'unknown' */
  liveliness: string;
  deadline_ns: number;
  lifespan_ns: number;
  liveliness_lease_duration_ns: number;
}
