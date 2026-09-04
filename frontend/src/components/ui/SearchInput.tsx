import type { InputHTMLAttributes } from 'react';

import { cn } from '../../lib/cn';
import { inputBase, inputMd } from '../../lib/uiStyles';

type SearchInputProps = InputHTMLAttributes<HTMLInputElement>;

export function SearchInput({ className, ...props }: SearchInputProps) {
  return (
    <input
      type="text"
      className={cn(inputBase, inputMd, 'w-full', className)}
      {...props}
    />
  );
}
