import { useCallback, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { mToMm } from '../../../constants/units';
import { cn } from '../../../lib/cn';
import { deleteMarkers, validateSize } from '../../../store/mapMutations.ts';
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

const inputClasses =
  'w-full rounded-row border border-line bg-surface-1 px-2 py-1.5 text-xs font-mono text-ink outline-none transition-colors duration-fast focus:border-accent/60 focus:ring-2 focus:ring-accent/20';

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
  /** Resolved value shown above each input (already formatted with its unit). */
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
              <div className="text-[10px] text-ink-faint text-center mb-0.5">{axis} = {resolved(ax)}</div>
              <input
                type="text"
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
                <div className="text-[10px] text-success mt-0.5 text-center">{preview}</div>
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
  const ui = useMapStore((s) => s.markerUi[String(marker.id)]);
  const setExpr = useMapStore((s) => s.setExpr);
  const setMarkerInfoLocal = useMapStore((s) => s.setMarkerInfoLocal);
  const setMutationError = useMapStore((s) => s.setMutationError);
  const deselectAll = useMapStore((s) => s.deselectAll);

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

  // Sync local state when the primary marker changes
  const [syncedMarkerId, setSyncedMarkerId] = useState(String(marker.id));
  if (String(marker.id) !== syncedMarkerId) {
    setSyncedMarkerId(String(marker.id));
    setLabelLocal(marker.markerFrameId);
    setSizeLocal(String(mToMm(marker.sizeM)));
    const nextUi = useMapStore.getState().markerUi[String(marker.id)];
    setPosExprLocal(nextUi ? [...nextUi.positionExpr] : ['0', '0', '0']);
    setRotExprLocal(nextUi ? [...nextUi.rotationExpr] : ['0', '0', '0']);
  }

  const primaryExprs = ui ?? {
    positionExpr: posExprLocal,
    rotationExpr: rotExprLocal,
  };
  const primaryPos = resolvePosition(primaryExprs.positionExpr, marker.id);
  const primaryRotEuler = quatToEulerDeg(resolveRotation(primaryExprs.rotationExpr, marker.id));

  const commitLabel = useCallback(() => {
    selectedIds.forEach((id) => setMarkerInfoLocal(id, { markerFrameId: labelLocal }));
  }, [selectedIds, labelLocal, setMarkerInfoLocal]);

  const commitSize = useCallback(() => {
    const val = parseFloat(sizeLocal);
    if (isNaN(val)) return;
    const error = validateSize(val / 1000);
    if (error !== null) {
      setMutationError(error);
      return;
    }
    selectedIds.forEach((id) => setMarkerInfoLocal(id, { sizeM: val / 1000 }));
  }, [selectedIds, sizeLocal, setMarkerInfoLocal, setMutationError]);

  const applyPosExpr = useCallback(
    (axis: 0 | 1 | 2) => {
      selectedIds.forEach((id) => setExpr(id, axis, 'position', posExprLocal[axis]));
    },
    [posExprLocal, selectedIds, setExpr],
  );

  const applyRotExpr = useCallback(
    (axis: 0 | 1 | 2) => {
      selectedIds.forEach((id) => setExpr(id, axis, 'rotation', rotExprLocal[axis]));
    },
    [rotExprLocal, selectedIds, setExpr],
  );

  const handleDelete = useCallback(() => {
    const label = isMulti ? t('map.deleteConfirmMany', { count: selectedIds.length }) : t('map.deleteConfirmOne', { id: marker.id });
    if (confirm(label)) {
      const ids = [...selectedIds];
      deselectAll();
      requestAnimationFrame(() => void deleteMarkers(ids));
    }
  }, [selectedIds, marker.id, deselectAll, isMulti, t]);

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
            <div className="text-[10px] text-ink-faint mt-1">{t('map.multiExprHint')}</div>
          </div>
        )}

        {/* Label */}
        <div>
          <div className="text-xs text-ink-muted mb-1">{t('map.label')}</div>
          <input
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
          <div className="text-xs text-ink-muted mb-1">{t('map.sizeMm')}</div>
          <input
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
