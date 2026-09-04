import type { ReactNode } from 'react';

import { SearchInput } from '../ui/SearchInput.tsx';

interface ListToolbarProps {
  value: string;
  onChange: (value: string) => void;
  placeholder: string;
  searchLabel?: string;
  extra?: ReactNode;
}

export function ListToolbar({
  value,
  onChange,
  placeholder,
  searchLabel,
  extra,
}: ListToolbarProps) {
  return (
    <div className="flex items-center gap-3 flex-wrap">
      <SearchInput
        value={value}
        onChange={(e) => onChange(e.target.value)}
        placeholder={placeholder}
        aria-label={searchLabel ?? placeholder}
        className="flex-1 min-w-48"
      />
      {extra}
    </div>
  );
}
