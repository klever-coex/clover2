export const DOC_PORT = 9000;

export function docsBaseUrl(lang: string): string {
  if (typeof window === 'undefined') {
    return `http://localhost:${DOC_PORT}/${lang}/`;
  }

  const { protocol, hostname } = window.location;

  return `${protocol}//${hostname}:${DOC_PORT}/${lang}/`;
}
