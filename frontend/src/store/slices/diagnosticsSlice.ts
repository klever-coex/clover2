import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import type { TopicSubscription } from '../../api/clover2.ts';
import {
  DIAG_HISTORY_MAX_POINTS,
  DIAG_HISTORY_WINDOW_MS,
  DIAGNOSTICS_TOPIC,
  LOGS_RECONNECT_BASE_DELAY_MS,
  LOGS_RECONNECT_MAX_DELAY_MS,
} from '../../constants/ros.ts';
import type { ApiError } from '@/types/errors';
import { parseDiagnosticArray } from '@/types/diagnostic.ts';
import type { DiagnosticEntry, DiagnosticPoint } from '@/types/diagnostic.ts';
import type { RosJsonValue } from '@/types/stream';
import type { RosStore } from '../useRosStore.ts';
import type { StreamState } from './streamSlice.ts';

/** Backend closes the stream with 1008 when the topic does not exist in the graph. */
const UNKNOWN_TOPIC_RETRY_DELAY_MS = 10_000;

function isUnknownTopicError(error: ApiError): boolean {
  return error.message.includes('unknown topic') || error.status === 1008;
}

export interface DiagnosticsSlice {
  /** Current diagnostics keyed by aggregated item name. */
  diagnostics: Record<string, DiagnosticEntry>;
  diagnosticsHistory: Record<string, DiagnosticPoint[]>;
  diagnosticsState: StreamState;
  /** True when the backend reported that DIAGNOSTICS_TOPIC has no publisher. */
  diagnosticsMissing: boolean;
  diagnosticsError: ApiError | null;
  diagnosticsReceived: number;
  diagnosticsPaused: boolean;
  diagnosticsDropped: number;
  startDiagnosticsStream: () => void;
  stopDiagnosticsStream: () => void;
  retryDiagnosticsStream: () => void;
  clearDiagnostics: () => void;
  setDiagnosticsPaused: (paused: boolean) => void;
}

function mergeEntries(
  existing: Record<string, DiagnosticEntry>,
  incoming: DiagnosticEntry[],
): Record<string, DiagnosticEntry> {
  const next = { ...existing };
  for (const entry of incoming) next[entry.name] = entry;
  return next;
}

function appendHistory(
  existing: Record<string, DiagnosticPoint[]>,
  incoming: DiagnosticEntry[],
  now: number,
  windowMs: number,
  cap: number,
): Record<string, DiagnosticPoint[]> {
  const next = { ...existing };
  for (const entry of incoming) {
    const points = [...(next[entry.name] ?? []), { t: now, levelValue: entry.levelValue }];
    let start = 0;
    while (start < points.length && (points[start]?.t ?? 0) < now - windowMs) start += 1;
    const pruned = points.slice(start);
    next[entry.name] = pruned.length > cap ? pruned.slice(-cap) : pruned;
  }
  return next;
}

export const createDiagnosticsSlice: StateCreator<RosStore, [], [], DiagnosticsSlice> = (set) => {
  let subscription: TopicSubscription | null = null;
  let reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  let failedAttempts = 0;

  const clearReconnectTimer = () => {
    if (reconnectTimer === null) return;
    clearTimeout(reconnectTimer);
    reconnectTimer = null;
  };

  const connect = () => {
    if (subscription !== null) return;

    set({ diagnosticsState: 'connecting' });

    subscription = clover2Api.topics.subscribe(DIAGNOSTICS_TOPIC, {
      onMessage: (message: RosJsonValue) => {
        const entries = parseDiagnosticArray(message);
        if (entries.length === 0) return;
        const now = Date.now();
        failedAttempts = 0;
        set((s) => ({
          diagnosticsState: 'connected',
          diagnosticsError: null,
          diagnosticsMissing: false,
          diagnosticsReceived: s.diagnosticsReceived + 1,
          ...(s.diagnosticsPaused
            ? { diagnosticsDropped: s.diagnosticsDropped + 1 }
            : {
                diagnostics: mergeEntries(s.diagnostics, entries),
                diagnosticsHistory: appendHistory(
                  s.diagnosticsHistory,
                  entries,
                  now,
                  DIAG_HISTORY_WINDOW_MS,
                  DIAG_HISTORY_MAX_POINTS,
                ),
              }),
        }));
      },

      onError: (error) => {
        subscription = null;
        if (isUnknownTopicError(error)) {
          // No publisher yet: wait quietly and retry on a slow cadence instead of spamming.
          set({ diagnosticsMissing: true, diagnosticsError: error });
          scheduleReconnect(UNKNOWN_TOPIC_RETRY_DELAY_MS, 'closed');
          return;
        }
        set({ diagnosticsError: error });
        scheduleReconnect();
      },

      onClose: () => {
        subscription = null;
        scheduleReconnect();
      },
    });
  };

  const scheduleReconnect = (delayOverride?: number, waitingState: StreamState = 'connecting') => {
    if (reconnectTimer !== null) return;
    const delay =
      delayOverride ??
      Math.min(LOGS_RECONNECT_BASE_DELAY_MS * (failedAttempts + 1), LOGS_RECONNECT_MAX_DELAY_MS);
    failedAttempts += 1;
    set({ diagnosticsState: waitingState });
    reconnectTimer = setTimeout(() => {
      reconnectTimer = null;
      connect();
    }, delay);
  };

  return {
    diagnostics: {},
    diagnosticsHistory: {},
    diagnosticsState: 'idle',
    diagnosticsMissing: false,
    diagnosticsError: null,
    diagnosticsReceived: 0,
    diagnosticsPaused: false,
    diagnosticsDropped: 0,

    startDiagnosticsStream: () => {
      if (reconnectTimer !== null) return; // a reconnect is already scheduled
      connect();
    },

    stopDiagnosticsStream: () => {
      clearReconnectTimer();
      subscription?.close();
      subscription = null;
      failedAttempts = 0;
      set({ diagnosticsState: 'idle' });
    },

    retryDiagnosticsStream: () => {
      clearReconnectTimer();
      failedAttempts = 0;
      subscription?.close();
      subscription = null;
      connect();
    },

    clearDiagnostics: () =>
      set({ diagnostics: {}, diagnosticsHistory: {}, diagnosticsDropped: 0, diagnosticsMissing: false }),

    setDiagnosticsPaused: (paused) => set({ diagnosticsPaused: paused, diagnosticsDropped: 0 }),
  };
};
