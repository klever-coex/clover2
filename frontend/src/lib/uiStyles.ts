/**
 * Shared Tailwind class tokens for UI primitives and page layout.
 * Keep these as the single source of truth so inputs, panels and grids
 * stay visually consistent without duplicating class strings inline.
 */

/** Base look of every text input / select trigger: border, focus ring, motion. */
export const inputBase =
  'rounded-row border border-line bg-surface-1 text-ink placeholder:text-ink-faint outline-none transition-colors duration-fast focus:border-accent/60 focus:ring-2 focus:ring-accent/20';

/** Dense input used in data-heavy panels (marker editor, settings rows). */
export const inputSm = 'px-2 py-1.5 text-xs font-mono';

/** Standard form input size. */
export const inputMd = 'px-3 py-2 text-sm';

/** Two-column workspace grid shared by the map and video pages. */
export const pageGrid = 'grid gap-3 h-full min-h-[420px]';

/** Panel body that fills the grid cell and collapses to 60vh on small screens. */
export const panelFill = 'h-[60vh] xl:h-full min-h-0';

/** Minimum height reserved for centered loading/error placeholders. */
export const centerFill = 'min-h-[60vh]';

/** Button color variants shared by Button and IconButton. */
export const buttonVariants = {
  primary: 'bg-accent text-white hover:bg-accent-hover active:bg-accent-active',
  secondary: 'bg-surface-2 text-ink border border-line hover:bg-surface-3',
  ghost: 'text-ink-muted hover:text-ink hover:bg-surface-2',
  danger: 'bg-error/15 text-error hover:bg-error/25',
} as const;
