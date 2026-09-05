import type { StateCreator } from 'zustand';

import { clover2Api } from '../../api/clover2.ts';
import type { TopicSubscription } from '../../api/clover2.ts';
import {
  LOGS_RECONNECT_BASE_DELAY_MS,
  LOGS_RECONNECT_MAX_DELAY_MS,
  LOG_BUFFER_CAP,
  ROSOUT_TOPIC,
  WS_KEEPALIVE_INTERVAL_MS,
} from '../../constants/ros.ts';
import type { ApiError } from '@/types/errors';
import { parseRosLogEntry } from '@/types/rosout';
import type { RosLogEntry } from '@/types/rosout';
import type { RosJsonValue } from '@/types/stream';
import type { RosStore } from '../useRosStore.ts';
import type { StreamState } from './streamSlice.ts';

export interface LogsSlice {
  logs: RosLogEntry[];
  logsState: StreamState;
  logsError: ApiError | null;
  logsReceived: number;
  logsPaused: boolean;
  logsDropped: number;
  startRosoutStream: () => void;
  retryRosoutStream: () => void;
  clearLogs: () => void;
  setLogsPaused: (paused: boolean) => void;
}

export const createLogsSlice: StateCreator<RosStore, [], [], LogsSlice> = (set) => {
  let subscription: TopicSubscription | null = null;
  let reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  let failedAttempts = 0;
  let seq = 0;

  const clearReconnectTimer = () => {
    if (reconnectTimer === null) return;
    clearTimeout(reconnectTimer);
    reconnectTimer = null;
  };

  const connect = () => {
    if (subscription !== null) return;

    set({ logsState: 'connecting' });

    subscription = clover2Api.topics.subscribe(ROSOUT_TOPIC, {
      keepaliveIntervalMs: WS_KEEPALIVE_INTERVAL_MS,

      onMessage: (message: RosJsonValue) => {
        const entry = parseRosLogEntry(message, seq++);
        if (entry === null) return;
        failedAttempts = 0;
        set((s) => ({
          logsState: 'connected',
          logsError: null,
          logsReceived: s.logsReceived + 1,
          ...(s.logsPaused
            ? { logsDropped: s.logsDropped + 1 }
            : { logs: [...s.logs, entry].slice(-LOG_BUFFER_CAP) }),
        }));
      },

      onError: (error) => {
        subscription = null;
        set({ logsError: error });
        scheduleReconnect();
      },

      onClose: () => {
        subscription = null;
        scheduleReconnect();
      },
    });
  };

  const scheduleReconnect = () => {
    if (reconnectTimer !== null) return;
    const delay = Math.min(
      LOGS_RECONNECT_BASE_DELAY_MS * (failedAttempts + 1),
      LOGS_RECONNECT_MAX_DELAY_MS,
    );
    failedAttempts += 1;
    set({ logsState: 'connecting' });
    reconnectTimer = setTimeout(() => {
      reconnectTimer = null;
      connect();
    }, delay);
  };

  return {
    logs: [],
    logsState: 'idle',
    logsError: null,
    logsReceived: 0,
    logsPaused: false,
    logsDropped: 0,

    startRosoutStream: () => {
      if (reconnectTimer !== null) return; // a reconnect is already scheduled
      connect();
    },

    retryRosoutStream: () => {
      clearReconnectTimer();
      failedAttempts = 0;
      subscription?.close();
      subscription = null;
      connect();
    },

    clearLogs: () => set({ logs: [], logsDropped: 0 }),

    setLogsPaused: (paused) => set({ logsPaused: paused, logsDropped: 0 }),
  };
};
