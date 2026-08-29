import { useCallback, useEffect, useId, useRef, useState } from 'react';
import type { KeyboardEvent as ReactKeyboardEvent } from 'react';
import { createPortal } from 'react-dom';
import { ChevronDown } from 'lucide-react';

import { cn } from '../../lib/cn.ts';

export interface SelectOption {
  value: string;
  label: string;
}

interface SelectProps {
  options: readonly SelectOption[];
  value: string;
  onChange: (value: string) => void;
  'aria-label'?: string;
  id?: string;
  className?: string;
  disabled?: boolean;
  direction?: 'down' | 'up';
}

export function Select({
  options,
  value,
  onChange,
  className,
  disabled,
  direction = 'down',
  ...labelProps
}: SelectProps) {
  const [isOpen, setIsOpen] = useState(false);
  const [activeIndex, setActiveIndex] = useState(0);
  const [menuPos, setMenuPos] = useState<{
    left: number;
    width: number;
    top?: number;
    bottom?: number;
  } | null>(null);
  const rootRef = useRef<HTMLDivElement>(null);
  const listRef = useRef<HTMLUListElement>(null);
  const itemRefs = useRef<(HTMLButtonElement | null)[]>([]);
  const listboxId = useId();

  const updateMenuPos = useCallback(() => {
    const rect = rootRef.current?.getBoundingClientRect();
    if (rect === undefined) return;
    setMenuPos(
      direction === 'up'
        ? { left: rect.left, width: rect.width, bottom: window.innerHeight - rect.top + 4 }
        : { left: rect.left, width: rect.width, top: rect.bottom + 4 },
    );
  }, [direction]);

  useEffect(() => {
    if (!isOpen) return;

    const onMouseDown = (event: MouseEvent) => {
      const target = event.target;
      if (!(target instanceof Node)) return;
      if (!rootRef.current?.contains(target) && !listRef.current?.contains(target)) {
        setIsOpen(false);
      }
    };
    const onKeyDown = (event: globalThis.KeyboardEvent) => {
      if (event.key === 'Escape') setIsOpen(false);
    };
    const onResizeScroll = () => updateMenuPos();

    document.addEventListener('mousedown', onMouseDown);
    document.addEventListener('keydown', onKeyDown);
    window.addEventListener('resize', onResizeScroll);
    window.addEventListener('scroll', onResizeScroll, true);
    return () => {
      document.removeEventListener('mousedown', onMouseDown);
      document.removeEventListener('keydown', onKeyDown);
      window.removeEventListener('resize', onResizeScroll);
      window.removeEventListener('scroll', onResizeScroll, true);
    };
  }, [isOpen, updateMenuPos]);

  useEffect(() => {
    if (isOpen) {
      itemRefs.current[activeIndex]?.scrollIntoView({ block: 'nearest' });
    }
  }, [isOpen, activeIndex]);

  const openMenu = () => {
    const selected = options.findIndex((option) => option.value === value);
    setActiveIndex(selected === -1 ? 0 : selected);
    updateMenuPos();
    setIsOpen(true);
  };

  const commit = (option: SelectOption) => {
    onChange(option.value);
    setIsOpen(false);
  };

  const handleKeyDown = (event: ReactKeyboardEvent) => {
    if (disabled) return;

    switch (event.key) {
      case 'ArrowDown':
        event.preventDefault();
        if (!isOpen) openMenu();
        else setActiveIndex((index) => (index + 1) % options.length);
        break;
      case 'ArrowUp':
        event.preventDefault();
        if (!isOpen) openMenu();
        else setActiveIndex((index) => (index - 1 + options.length) % options.length);
        break;
      case 'Enter':
      case ' ':
        event.preventDefault();
        if (!isOpen) openMenu();
        else {
          const option = options[activeIndex];
          if (option !== undefined) commit(option);
        }
        break;
      case 'Tab':
        setIsOpen(false);
        break;
    }
  };

  const selected = options.find((option) => option.value === value);
  const activeOption = options[activeIndex];

  return (
    <div ref={rootRef} className={cn('relative', className)}>
      <button
        type="button"
        id={labelProps.id}
        aria-label={labelProps['aria-label']}
        aria-haspopup="listbox"
        aria-expanded={isOpen}
        aria-controls={listboxId}
        disabled={disabled}
        onClick={() => (isOpen ? setIsOpen(false) : openMenu())}
        onKeyDown={handleKeyDown}
        className={cn(
          'w-full flex items-center justify-between gap-2 cursor-pointer rounded-row border border-line bg-surface-1 px-3 py-2 text-sm text-ink text-left outline-none transition-colors duration-fast focus:border-accent/60 focus:ring-2 focus:ring-accent/20 disabled:opacity-50 disabled:pointer-events-none',
          isOpen && 'border-accent/60',
        )}
      >
        <span className="truncate">{selected?.label ?? ''}</span>
        <ChevronDown
          size={16}
          className={cn(
            'shrink-0 text-ink-muted transition-transform duration-normal',
            isOpen && 'rotate-180',
          )}
        />
      </button>

      {isOpen &&
        menuPos !== null &&
        createPortal(
          <ul
            ref={listRef}
            id={listboxId}
            role="listbox"
            aria-activedescendant={
              activeOption === undefined ? undefined : `${listboxId}-${activeIndex}`
            }
            style={{
              position: 'fixed',
              left: menuPos.left,
              width: menuPos.width,
              top: menuPos.top,
              bottom: menuPos.bottom,
            }}
            className="z-50 max-h-64 overflow-y-auto rounded-panel border border-line bg-surface-1 p-1 shadow-lg"
          >
            {options.map((option, index) => {
              const isSelected = option.value === value;

              return (
                <li key={option.value} role="none">
                  <button
                    type="button"
                    ref={(element) => {
                      itemRefs.current[index] = element;
                    }}
                    id={`${listboxId}-${index}`}
                    role="option"
                    aria-selected={isSelected}
                    tabIndex={-1}
                    onClick={() => commit(option)}
                    onMouseEnter={() => setActiveIndex(index)}
                    className={cn(
                      'w-full flex items-center justify-between gap-2 px-3 py-2 rounded-row text-sm text-left transition-colors duration-fast',
                      index === activeIndex && 'bg-surface-2',
                      isSelected ? 'text-accent-text' : 'text-ink',
                    )}
                  >
                    <span className="truncate">{option.label}</span>
                  </button>
                </li>
              );
            })}
          </ul>,
          document.body,
        )}
    </div>
  );
}
