import { useEffect, useId, useRef } from 'react';
import { createPortal } from 'react-dom';
import { useTranslation } from 'react-i18next';

import { useConfirmStore } from '../../store/useConfirmStore.ts';
import { cn } from '../../lib/cn.ts';
import { Button } from './Button.tsx';


export function ConfirmDialog() {
  const { t } = useTranslation();
  const options = useConfirmStore((s) => s.options);
  const settle = useConfirmStore((s) => s.settle);

  const dialogRef = useRef<HTMLDivElement>(null);
  const cancelRef = useRef<HTMLButtonElement>(null);
  const restoreFocusRef = useRef<HTMLElement | null>(null);
  const titleId = useId();
  const messageId = useId();

  const isOpen = options !== null;

  useEffect(() => {
    if (!isOpen) return;

    restoreFocusRef.current =
      document.activeElement instanceof HTMLElement ? document.activeElement : null;
    cancelRef.current?.focus();

    const onKeyDown = (event: KeyboardEvent) => {
      if (event.key === 'Escape') {
        event.preventDefault();
        settle(false);
        return;
      }
      if (event.key !== 'Tab') return;
      const focusable = dialogRef.current?.querySelectorAll<HTMLElement>('button:not([disabled])');
      if (focusable === undefined || focusable.length === 0) return;
      const first = focusable[0]!;
      const last = focusable[focusable.length - 1]!;
      if (event.shiftKey && document.activeElement === first) {
        event.preventDefault();
        last.focus();
      } else if (!event.shiftKey && document.activeElement === last) {
        event.preventDefault();
        first.focus();
      }
    };

    document.addEventListener('keydown', onKeyDown, true);
    return () => {
      document.removeEventListener('keydown', onKeyDown, true);
      restoreFocusRef.current?.focus();
    };
  }, [isOpen, settle]);

  if (!isOpen || options === null) return null;

  const isDanger = options.tone === 'danger';

  return createPortal(
    <div
      className="fixed inset-0 z-[100] flex items-center justify-center bg-black/50 p-4"
      onMouseDown={(e) => {
        if (e.target === e.currentTarget) settle(false);
      }}
    >
      <div
        ref={dialogRef}
        role="dialog"
        aria-modal="true"
        aria-labelledby={titleId}
        aria-describedby={messageId}
        className="w-full max-w-sm rounded-panel border border-line bg-surface-1 p-4 shadow-lg"
      >
        {options.title !== undefined && (
          <h2 id={titleId} className="text-sm font-semibold text-ink">
            {options.title}
          </h2>
        )}
        <p
          id={messageId}
          className={cn('text-sm text-ink-muted', options.title !== undefined && 'mt-2')}
        >
          {options.message}
        </p>
        <div className="mt-4 flex justify-end gap-2">
          <Button ref={cancelRef} variant="secondary" size="sm" onClick={() => settle(false)}>
            {options.cancelLabel ?? t('common.cancel')}
          </Button>
          <Button
            variant={isDanger ? 'danger' : 'primary'}
            size="sm"
            onClick={() => settle(true)}
          >
            {options.confirmLabel ?? t('common.confirm')}
          </Button>
        </div>
      </div>
    </div>,
    document.body,
  );
}
