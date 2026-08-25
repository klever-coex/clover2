import type { ReactNode } from 'react';

import { SearchInput } from '../ui/SearchInput.tsx';

interface ListToolbarProps {
  value: string;
  onChange: (value: string) => void;
  placeholder: string;
  /** Optional control (e.g. SortSelect) rendered next to the search input. */
  extra?: ReactNode;
}

/** Search input + optional extra controls, shared by list pages. */
export function ListToolbar({ value, onChange, placeholder, extra }: ListToolbarProps) {
  return (
    <div className="flex items-center gap-3 flex-wrap">
      <SearchInput
        value={value}
        onChange={(e) => onChange(e.target.value)}
        placeholder={placeholder}
        className="flex-1 min-w-48"
      />
      {extra}
    </div>
  );
}
