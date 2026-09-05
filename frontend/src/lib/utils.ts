import { clsx, type ClassValue } from 'clsx';
import { extendTailwindMerge } from 'tailwind-merge';

/**
 * tailwind-merge must know our custom tokens, otherwise it keeps conflicting
 * classes (e.g. `rounded-xl rounded-panel`) or drops siblings from the same
 * namespace (font-size vs text-color) and CSS source order decides the winner.
 * RULE: every new custom token added to @theme in index.css must be registered
 * here in its matching class group.
 */
const twMerge = extendTailwindMerge({
  extend: {
    classGroups: {
      rounded: [{ rounded: ['panel', 'row'] }],
      'font-size': [{ text: ['micro'] }],
      'text-color': [
        {
          text: [
            'primary',
            'secondary',
            'destructive',
            'success',
            'warning',
            'info',
            'foreground',
            'muted-foreground',
            'json-key',
            'json-string',
            'json-null',
          ],
        },
      ],
      'bg-color': [
        {
          bg: [
            'primary',
            'secondary',
            'destructive',
            'success',
            'warning',
            'info',
            'card',
            'popover',
            'muted',
            'accent',
            'background',
            'sidebar',
            'sidebar-accent',
          ],
        },
      ],
      duration: [{ duration: ['fast', 'normal', 'slow'] }],
    },
  },
});

export function cn(...inputs: ClassValue[]): string {
  return twMerge(clsx(inputs));
}
