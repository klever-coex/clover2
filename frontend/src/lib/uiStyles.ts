/**
 * Shared Tailwind class tokens for page layout and input density.
 * Component look-and-feel lives in the shadcn components (src/components/ui/*.tsx);
 * these helpers only cover layout geometry and dense-input sizing.
 */

/** Dense input size used in data-heavy panels (marker editor, settings rows). */
export const inputSm = 'px-2 py-1.5 text-xs font-mono';

/** Two-column workspace grid shared by the map and video pages. */
export const pageGrid = 'grid gap-3 h-full min-h-[420px]';

/** Panel body that fills the grid cell and collapses to 60vh on small screens. */
export const panelFill = 'h-[60vh] xl:h-full min-h-0';

/** Minimum height reserved for centered loading/error placeholders. */
export const centerFill = 'min-h-[60vh]';
