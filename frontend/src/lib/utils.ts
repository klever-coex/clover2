import { clsx, type ClassValue } from 'clsx';
import { extendTailwindMerge } from 'tailwind-merge';

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
      duration: [{ duration: ['fast', 'normal'] }],
    },
  },
});

export function cn(...inputs: ClassValue[]): string {
  return twMerge(clsx(inputs));
}
