import { Plus } from 'lucide-react';
import { useId, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { addMarker, suggestMarkerId, validateMarkerId } from '../../../store/mapMutations.ts';
import { DEFAULT_DICTIONARY } from '@/constants/defaults';
import { maxMarkerId } from '../../../data/dictionaries/index.ts';
import { cn } from '@/lib/utils';
import { inputSm } from '@/lib/uiStyles';
import { useMapStore } from '@/store/useMapStore';
import { useMapUIStore } from '@/store/useMapUIStore';
import {
  AlertDialog,
  AlertDialogAction,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
  AlertDialogTrigger,
} from '@/components/ui/alert-dialog.tsx';
import { Button } from '@/components/ui/button';
import { Field, FieldError, FieldLabel } from '@/components/ui/field';
import { Input } from '@/components/ui/input';

export function AddMarkerButton() {
  const { t } = useTranslation();
  const idInputId = useId();
  const dictionary = useMapStore((s) => s.mapMeta?.dictionary ?? DEFAULT_DICTIONARY);
  const frameId = useMapStore((s) => s.mapMeta?.frameId ?? '');
  const [open, setOpen] = useState(false);
  const [idText, setIdText] = useState('');
  const [exhausted, setExhausted] = useState(false);

  const maxId = maxMarkerId(dictionary);

  const handleOpenChange = (next: boolean) => {
    if (next) {
      const suggested = suggestMarkerId();
      setIdText(suggested !== null ? String(suggested) : '');
      setExhausted(suggested === null);
    }
    useMapUIStore.getState().setModalOpen(next);
    setOpen(next);
  };

  const error =
    idText.trim() === ''
      ? exhausted
        ? t('map.idExhausted')
        : null
      : validateMarkerId(Number(idText));
  const canAdd = idText.trim() !== '' && error === null;
  const namePreview = canAdd ? `${frameId || 'map'}_aruco_${Number(idText)}` : null;

  const confirm = () => {
    if (!canAdd) return;
    addMarker(Number(idText));
  };

  return (
    <AlertDialog open={open} onOpenChange={handleOpenChange}>
      <AlertDialogTrigger asChild>
        <Button>
          <Plus />
          {t('map.addMarker')}
        </Button>
      </AlertDialogTrigger>
      <AlertDialogContent>
        <AlertDialogHeader>
          <AlertDialogTitle>{t('map.addMarkerTitle')}</AlertDialogTitle>
          <AlertDialogDescription>
            {t('map.addMarkerHint', { dictionary, max: maxId })}
          </AlertDialogDescription>
        </AlertDialogHeader>
        <Field data-invalid={error !== null}>
          <FieldLabel htmlFor={idInputId} className="text-xs text-muted-foreground">
            {t('map.markerId')}
          </FieldLabel>
          <Input
            id={idInputId}
            type="number"
            value={idText}
            min={0}
            max={maxId}
            step={1}
            aria-invalid={error !== null}
            onChange={(e) => setIdText(e.target.value)}
            onKeyDown={(e) => {
              if (e.key === 'Enter' && canAdd) {
                confirm();
                setOpen(false);
              }
            }}
            className={cn(inputSm, 'w-full')}
          />
          {namePreview !== null && (
            <p className="truncate text-xs text-muted-foreground">
              {t('map.namePreview', { name: namePreview })}
            </p>
          )}
          <FieldError>{error}</FieldError>
        </Field>
        <AlertDialogFooter>
          <AlertDialogCancel>{t('common.cancel')}</AlertDialogCancel>
          <AlertDialogAction disabled={!canAdd} onClick={confirm}>
            {t('map.add')}
          </AlertDialogAction>
        </AlertDialogFooter>
      </AlertDialogContent>
    </AlertDialog>
  );
}
