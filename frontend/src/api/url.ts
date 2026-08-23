import { HTTP_PORT } from '../constants/ros.ts';

export function resolveBaseUrl(): string {
  const { protocol, hostname } = window.location;
  return `${protocol}//${hostname}:${HTTP_PORT}`;
}

export function toWebSocketBase(baseUrl: string): string {
  return baseUrl.replace(/^http/, 'ws');
}

export function encodeRosPath(name: string): string {
  return name
    .replace(/^\/+/, '')
    .split('/')
    .map(encodeURIComponent)
    .join('/');
}
