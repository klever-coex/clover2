import { useCallback, useId, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { mToMm } from '../../../constants/units';
import { cn } from '../../../lib/cn';
import { inputBase, inputSm } from '../../../lib/uiStyles';
import { deleteMarkersAfterDetach, validateSize } from '../../../store/mapMutations.ts';
import { confirmDialog } from '../../../store/useConfirmStore.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import type { MapMarker } from '../../../types/marker.ts';
import { evaluateExpr } from '../../../utils/exprEval.ts';
import { quatToEulerDeg, resolvePosition, resolveRotation } from '../../../utils/transformUtils.ts';
import { Button } from '../../ui/Button.tsx';
import { Panel } from '../../ui/Panel.tsx';
import { MarkerTypeBadge } from './MarkerTypeBadge.tsx';

interface Props {
  marker: MapMarker;
  selectedIds: string[];
}

const inputClasses = cn(inputBase, inputSm, 'w-full');

/** Formats an expression for the hint under the input, or null when it does not evaluate. */
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
  onApply: (axis: 0 | 1 | 2) => void;
}

/** Three X/Y/Z expression inputs sharing label, preview and commit behaviour. */
function ExprAxisRow({ label, unit, values, resolved, markerId, onChange, onApply }: ExprAxisRowProps) {
  return (
    <div>
      <div className="text-xs text-ink-muted mb-1">{label}</div>
      <div className="flex gap-1">
        {(['X', 'Y', 'Z'] as const).map((axis, i) => {
          const ax = i as 0 | 1 | 2;
          const preview = exprPreview(values[ax], markerId, unit);
          return (
            <div key={axis} className="flex-1">
              <div className="text-micro text-ink-faint text-center mb-0.5">{axis} = {resolved(ax)}</div>
              <input
                type="text"
                aria-label={`${label}: ${axis}`}
                value={values[ax]}
                onChange={(e) => onChange(ax, e.target.value)}
                onKeyDown={(e) => {
                  if (e.key === 'Enter') onApply(ax);
                }}
                onBlur={() => onApply(ax)}
                placeholder={`${axis.toLowerCase()} expr`}
                className={cn(inputClasses, 'text-center')}
              />
              {preview && (
                <div className="text-micro text-success mt-0.5 text-center">{preview}</div>
              )}
            </div>
          );
        })}
      </div>
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

  const primaryExprs = ui ?? {
    positionExpr: posExprLocal,
    rotationExpr: rotExprLocal,
  };
  const primaryPos = resolvePosition(primaryExprs.positionExpr, marker.id);
  const primaryRotEuler = quatToEulerDeg(resolveRotation(primaryExprs.rotationExpr, marker.id));

  /** Applies a change to every marker in the selection. */
  const commitSelected = useCallback(
    (apply: (id: string) => void) => {
      selectedIds.forEach(apply);
    },
    [selectedIds],
  );

  const commitLabel = useCallback(() => {
    commitSelected((id) => setMarkerInfoLocal(id, { markerFrameId: labelLocal }));
  }, [commitSelected, labelLocal, setMarkerInfoLocal]);

  const commitSize = useCallback(() => {
    const val = parseFloat(sizeLocal);
    if (isNaN(val)) return;
    const error = validateSize(val / 1000);
    if (error !== null) {
      setMutationError(error);
      return;
    }
    commitSelected((id) => setMarkerInfoLocal(id, { sizeM: val / 1000 }));
  }, [commitSelected, sizeLocal, setMarkerInfoLocal, setMutationError]);

  const applyPosExpr = useCallback(
    (axis: 0 | 1 | 2) => {
      commitSelected((id) => setExpr(id, axis, 'position', posExprLocal[axis]));
    },
    [commitSelected, posExprLocal, setExpr],
  );

  const applyRotExpr = useCallback(
    (axis: 0 | 1 | 2) => {
      commitSelected((id) => setExpr(id, axis, 'rotation', rotExprLocal[axis]));
    },
    [commitSelected, rotExprLocal, setExpr],
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
    <Panel
      padded={false}
      title={
        <span className="font-mono">
          #{marker.id} {marker.markerFrameId}
        </span>
      }
      actions={<MarkerTypeBadge type={marker.type} />}
    >
      <div className="p-4 space-y-3">
        {/* Multi-selection banner */}
        {isMulti && (
          <div className="rounded-row bg-accent-soft px-3 py-2 text-center">
            <div className="text-xs font-semibold text-accent-text">
              {t('map.editingMany', { count: selectedIds.length })}
            </div>
            <div className="text-micro text-ink-faint mt-1">{t('map.multiExprHint')}</div>
          </div>
        )}

        {/* Label */}
        <div>
          <label htmlFor={labelInputId} className="text-xs text-ink-muted mb-1 block">
            {t('map.label')}
          </label>
          <input
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
        </div>

        {/* Size */}
        <div>
          <label htmlFor={sizeInputId} className="text-xs text-ink-muted mb-1 block">
            {t('map.sizeMm')}
          </label>
          <input
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
        </div>

        {!canEditPose && (
          <p className="text-xs text-ink-faint">{t('map.poseUnknown')}</p>
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
              resolved={(axis) => `${primaryRotEuler[axis].toFixed(1)}°`}
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
        <Button variant="danger" className="w-full" onClick={handleDelete}>
          {isMulti ? t('map.deleteMany', { count: selectedIds.length }) : t('map.delete')}
        </Button>
      </div>
    </Panel>
  );
}
