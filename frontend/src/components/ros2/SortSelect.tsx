import { Select } from '../ui/Select.tsx';
import type { SelectOption } from '../ui/Select.tsx';

interface SortSelectProps {
  ariaLabel: string;
  value: string;
  onChange: (value: string) => void;
  options: readonly SelectOption[];
}

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
