export const HTTP_PORT = 3000;
export const STREAM_BUFFER_CAP = 20;

/** Read requests: fail fast so an unreachable backend never leaves a spinner hanging. */
export const REQUEST_TIMEOUT_MS = 10_000;
/** Writes may re-solve the map on the drone; aborting one risks a half-applied change. */
export const MUTATION_TIMEOUT_MS = 30_000;
/** A stalled WebSocket handshake fires neither onclose nor onerror. */
export const WS_CONNECT_TIMEOUT_MS = 10_000;
