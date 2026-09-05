import type { ReactNode } from 'react';
import { SearchIcon } from 'lucide-react';

import { InputGroup, InputGroupAddon, InputGroupInput } from '@/components/ui/input-group';

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
      <InputGroup className="w-full flex-1 min-w-48">
        <InputGroupAddon align="inline-start">
          <SearchIcon />
        </InputGroupAddon>
        <InputGroupInput
          type="search"
          value={value}
          onChange={(e) => onChange(e.target.value)}
          placeholder={placeholder}
          aria-label={searchLabel ?? placeholder}
        />
      </InputGroup>
      {extra}
    </div>
  );
}
