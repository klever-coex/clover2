import { useEffect, useRef } from 'react';
import { useTranslation } from 'react-i18next';

import { useConfirmStore } from '@/store/useConfirmStore';
import {
  AlertDialog,
  AlertDialogAction,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
} from '@/components/ui/alert-dialog.tsx';

export function ConfirmDialog() {
  const { t } = useTranslation();
  const options = useConfirmStore((s) => s.options);
  const settle = useConfirmStore((s) => s.settle);

  const lastOptions = useRef(options);
  if (options !== null) lastOptions.current = options;

  useEffect(() => () => useConfirmStore.getState().settle(false), []);

  const isOpen = options !== null;
  const shown = options ?? lastOptions.current;

  return (
    <AlertDialog
      open={isOpen}
      onOpenChange={(open) => {
        if (!open) settle(false);
      }}
    >
      <AlertDialogContent className="max-w-sm">
        <AlertDialogHeader>
          <AlertDialogTitle>{shown?.title ?? t('common.confirm')}</AlertDialogTitle>
          <AlertDialogDescription>{shown?.message}</AlertDialogDescription>
        </AlertDialogHeader>
        <AlertDialogFooter>
          <AlertDialogCancel>{shown?.cancelLabel ?? t('common.cancel')}</AlertDialogCancel>
          <AlertDialogAction
            variant={shown?.tone === 'danger' ? 'destructive' : 'default'}
            onClick={() => settle(true)}
          >
            {shown?.confirmLabel ?? t('common.confirm')}
          </AlertDialogAction>
        </AlertDialogFooter>
      </AlertDialogContent>
    </AlertDialog>
  );
}
