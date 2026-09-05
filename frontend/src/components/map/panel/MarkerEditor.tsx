import { useCallback, useId, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { mToMm } from '@/constants/units';
import { cn } from '@/lib/utils';
import { inputSm } from '@/lib/uiStyles';
import { deleteMarkersAfterDetach, validateSize } from '../../../store/mapMutations.ts';
import { confirmDialog } from '@/store/useConfirmStore';
import { useMapStore } from '@/store/useMapStore';
import type { MapMarker } from '@/types/marker';
import { evaluateExpr } from '../../../utils/exprEval.ts';
import { quatToEulerDeg, resolvePosition, resolveRotation } from '../../../utils/transformUtils.ts';
import { Button } from '@/components/ui/button';
import { Card, CardAction, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Field, FieldLabel } from '@/components/ui/field';
import { Input } from '@/components/ui/input';
import { MarkerTypeBadge } from './MarkerTypeBadge.tsx';

interface Props {
  marker: MapMarker;
  selectedIds: string[];
}

const inputClasses = cn(inputSm, 'w-full');

function exprPreview(expr: string, markerId: number, unit: string): string | null {
  if (!expr.trim()) return null;
  try {
    const value = evaluateExpr(expr, { id: markerId });
    if (!Number.isFinite(value)) return null;
    return `${value.toFixed(1)} ${unit}`;
  } catch {
    return null;
  }
}

interface ExprAxisRowProps {
  label: string;
  unit: string;
  values: [string, string, string];
  resolved: (axis: 0 | 1 | 2) => string;
  markerId: number;
  onChange: (axis: 0 | 1 | 2, value: string) => void;
  onApply: (axis: 0 | 1 | 2) => string | null;
}

function ExprAxisRow({ label, unit, values, resolved, markerId, onChange, onApply }: ExprAxisRowProps) {
  const [error, setError] = useState<string | null>(null);

  const apply = (axis: 0 | 1 | 2) => setError(onApply(axis));

  return (
    <div className="flex flex-col gap-1">
      <div className="text-xs text-muted-foreground">{label}</div>
      {(['X', 'Y', 'Z'] as const).map((axis, i) => {
        const ax = i as 0 | 1 | 2;
        const preview = exprPreview(values[ax], markerId, unit);
        return (
          <div key={axis} className="flex items-center gap-1.5">
            <span className="w-3 shrink-0 text-center text-micro font-semibold text-muted-foreground">
              {axis}
            </span>
            <Input
              type="text"
              aria-label={`${label}: ${axis}`}
              value={values[ax]}
              onChange={(e) => onChange(ax, e.target.value)}
              onKeyDown={(e) => {
                if (e.key === 'Enter') apply(ax);
              }}
              onBlur={() => apply(ax)}
              placeholder={`${axis.toLowerCase()} = f(id)`}
              className={cn(inputClasses, 'pr-1')}
            />
            <span
              title={preview ?? undefined}
              className={cn(
                'w-12 shrink-0 truncate text-right text-micro tabular-nums',
                preview !== null ? 'text-success' : 'text-muted-foreground/80',
              )}
            >
              {resolved(ax)}
            </span>
          </div>
        );
      })}
      {error !== null && (
        <div className="text-micro text-destructive" role="alert">
          {error}
        </div>
      )}
    </div>
  );
}

export function MarkerEditor({ marker, selectedIds }: Props) {
  const { t } = useTranslation();
  const labelInputId = useId();
  const sizeInputId = useId();
  const ui = useMapStore((s) => s.markerUi[String(marker.id)]);
  const setExpr = useMapStore((s) => s.setExpr);
  const setMarkerInfoLocal = useMapStore((s) => s.setMarkerInfoLocal);
  const setMutationError = useMapStore((s) => s.setMutationError);

  const isMulti = selectedIds.length > 1;
  const canEditPose = marker.type === 'fixed' && marker.pose !== null;

  const [labelLocal, setLabelLocal] = useState(marker.markerFrameId);
  const [sizeLocal, setSizeLocal] = useState(String(mToMm(marker.sizeM)));
  const [posExprLocal, setPosExprLocal] = useState<[string, string, string]>(
    () => (ui ? [...ui.positionExpr] : ['0', '0', '0']),
  );
  const [rotExprLocal, setRotExprLocal] = useState<[string, string, string]>(
    () => (ui ? [...ui.rotationExpr] : ['0', '0', '0']),
  );

  const [prevStorePos, setPrevStorePos] = useState(ui?.positionExpr);
  const [prevStoreRot, setPrevStoreRot] = useState(ui?.rotationExpr);
  if (ui !== undefined && ui.positionExpr !== prevStorePos) {
    setPrevStorePos(ui.positionExpr);
    setPosExprLocal([...ui.positionExpr]);
  }
  if (ui !== undefined && ui.rotationExpr !== prevStoreRot) {
    setPrevStoreRot(ui.rotationExpr);
    setRotExprLocal([...ui.rotationExpr]);
  }

  const primaryExprs = ui ?? {
    positionExpr: posExprLocal,
    rotationExpr: rotExprLocal,
  };
  const primaryPos = resolvePosition(primaryExprs.positionExpr, marker.id);
  const primaryRotEuler = quatToEulerDeg(resolveRotation(primaryExprs.rotationExpr, marker.id));

  const commitSelected = useCallback(
    (apply: (id: string) => void) => {
      selectedIds.forEach(apply);
    },
    [selectedIds],
  );

  const commitLabel = useCallback(() => {
    if (labelLocal === marker.markerFrameId) return;
    commitSelected((id) => setMarkerInfoLocal(id, { markerFrameId: labelLocal }));
  }, [commitSelected, labelLocal, marker.markerFrameId, setMarkerInfoLocal]);

  const commitSize = useCallback(() => {
    const val = parseFloat(sizeLocal);
    if (isNaN(val)) return;
    if (val / 1000 === marker.sizeM) return;
    const error = validateSize(val / 1000);
    if (error !== null) {
      setMutationError(error);
      return;
    }
    commitSelected((id) => setMarkerInfoLocal(id, { sizeM: val / 1000 }));
  }, [commitSelected, sizeLocal, marker.sizeM, setMarkerInfoLocal, setMutationError]);

  const applyExpr = useCallback(
    (axis: 0 | 1 | 2, kind: 'position' | 'rotation', expr: string): string | null => {
      try {
        evaluateExpr(expr, { id: marker.id });
      } catch (error) {
        return (error as Error).message;
      }
      commitSelected((id) => setExpr(id, axis, kind, expr));
      return null;
    },
    [commitSelected, marker.id, setExpr],
  );

  const applyPosExpr = useCallback(
    (axis: 0 | 1 | 2) => applyExpr(axis, 'position', posExprLocal[axis]),
    [applyExpr, posExprLocal],
  );

  const applyRotExpr = useCallback(
    (axis: 0 | 1 | 2) => applyExpr(axis, 'rotation', rotExprLocal[axis]),
    [applyExpr, rotExprLocal],
  );

  const handleDelete = useCallback(async () => {
    const message = isMulti
      ? t('map.deleteConfirmMany', { count: selectedIds.length })
      : t('map.deleteConfirmOne', { id: marker.id });
    const confirmed = await confirmDialog({
      message,
      tone: 'danger',
      confirmLabel: t('map.delete'),
    });
    if (confirmed) {
      deleteMarkersAfterDetach([...selectedIds]);
    }
  }, [selectedIds, marker.id, isMulti, t]);

  return (
    <Card size="sm">
      <CardHeader>
        <CardTitle className="font-mono">
          #{marker.id} {marker.markerFrameId}
        </CardTitle>
        <CardAction>
          <MarkerTypeBadge type={marker.type} />
        </CardAction>
      </CardHeader>
      <CardContent className="flex flex-col gap-3">
        {/* Multi-selection banner */}
        {isMulti && (
          <div className="rounded-row bg-primary/10 px-3 py-2 text-center">
            <div className="text-xs font-semibold text-primary">
              {t('map.editingMany', { count: selectedIds.length })}
            </div>
            <div className="text-micro text-muted-foreground/80 mt-1">{t('map.multiExprHint')}</div>
          </div>
        )}

        {/* Label */}
        <Field>
          <FieldLabel htmlFor={labelInputId} className="text-xs text-muted-foreground">
            {t('map.label')}
          </FieldLabel>
          <Input
            id={labelInputId}
            type="text"
            value={labelLocal}
            onChange={(e) => setLabelLocal(e.target.value)}
            onKeyDown={(e) => {
              if (e.key === 'Enter') commitLabel();
            }}
            onBlur={commitLabel}
            className={inputClasses}
          />
        </Field>

        {/* Size */}
        <Field>
          <FieldLabel htmlFor={sizeInputId} className="text-xs text-muted-foreground">
            {t('map.sizeMm')}
          </FieldLabel>
          <Input
            id={sizeInputId}
            type="number"
            value={sizeLocal}
            onChange={(e) => setSizeLocal(e.target.value)}
            onKeyDown={(e) => {
              if (e.key === 'Enter') commitSize();
            }}
            onBlur={commitSize}
            min={10}
            step={10}
            className={inputClasses}
          />
        </Field>

        {!canEditPose && (
          <p className="text-xs text-muted-foreground/80">{t('map.poseUnknown')}</p>
        )}

        {canEditPose && (
          <>
            <ExprAxisRow
              label={t('map.positionExpr')}
              unit="mm"
              values={posExprLocal}
              markerId={marker.id}
              resolved={(axis) => `${mToMm(primaryPos[axis]).toFixed(1)}`}
              onChange={(axis, value) =>
                setPosExprLocal((prev) => {
                  const next: [string, string, string] = [...prev];
                  next[axis] = value;
                  return next;
                })
              }
              onApply={applyPosExpr}
            />

            <ExprAxisRow
              label={t('map.rotationExpr')}
              unit="°"
              values={rotExprLocal}
              markerId={marker.id}
              resolved={(axis) => `${primaryRotEuler[axis].toFixed(1)}`}
              onChange={(axis, value) =>
                setRotExprLocal((prev) => {
                  const next: [string, string, string] = [...prev];
                  next[axis] = value;
                  return next;
                })
              }
              onApply={applyRotExpr}
            />
          </>
        )}

        {/* Delete */}
        <Button variant="destructive" className="w-full" onClick={handleDelete}>
          {isMulti ? t('map.deleteMany', { count: selectedIds.length }) : t('map.delete')}
        </Button>
      </CardContent>
    </Card>
  );
}
