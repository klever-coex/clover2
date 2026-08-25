import { Select } from '../ui/Select.tsx';
import type { SelectOption } from '../ui/Select.tsx';

interface SortSelectProps {
  /** Accessible name for the sort control. */
  ariaLabel: string;
  value: string;
  onChange: (value: string) => void;
  options: readonly SelectOption[];
}

/** Sort select styled and sized for list-page toolbars. */
export function SortSelect({ ariaLabel, value, onChange, options }: SortSelectProps) {
  return (
    <Select
      aria-label={ariaLabel}
      value={value}
      onChange={onChange}
      options={options}
      className="w-48"
    />
  );
}
