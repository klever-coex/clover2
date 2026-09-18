import { HTTP_PORT } from '../constants/ros.ts';

export function resolveBaseUrl(): string {
  if (typeof window === 'undefined') {
    return `http://localhost:${HTTP_PORT}`;
  }
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
    .filter((segment) => segment !== '' && segment !== '.' && segment !== '..')
    .map(encodeURIComponent)
    .join('/');
}
