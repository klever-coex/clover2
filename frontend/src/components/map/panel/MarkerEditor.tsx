import { useCallback, useState } from 'react';
import { useTranslation } from 'react-i18next';

import type { TranslationKey } from '../../../i18n/index.ts';
import { mToMm } from '../../../constants/units.ts';
import { applyPose, deleteMarkers, updateMarkerInfo } from '../../../pages/map/mutations.ts';
import { useMapStore } from '../../../store/useMapStore.ts';
import type { MapMarker, MarkerType } from '../../../types/marker.ts';
import { evaluateExpr } from '../../../utils/exprEval.ts';
import { quatToEulerDeg, resolvePosition, resolveRotation } from '../../../utils/transformUtils.ts';
import { Badge } from '../../ui/Badge.tsx';
import type { BadgeTone } from '../../ui/Badge.tsx';
import { Button } from '../../ui/Button.tsx';
import { Panel } from '../../ui/Panel.tsx';

interface Props {
  marker: MapMarker;
  selectedIds: string[];
}

const TYPE_TONES: Record<MarkerType, BadgeTone> = {
  fixed: 'accent',
  static: 'neutral',
  dynamic: 'warning',
};

const TYPE_KEYS: Record<MarkerType, TranslationKey> = {
  fixed: 'map.typeFixed',
  static: 'map.typeStatic',
  dynamic: 'map.typeDynamic',
};

const inputClasses =
  'w-full rounded-row border border-line bg-surface-1 px-2 py-1.5 text-xs font-mono text-ink outline-none transition-colors duration-fast focus:border-accent/60 focus:ring-2 focus:ring-accent/20';

/** Preview the result of evaluating a position expression for a single marker's markerId */
function posPreview(expr: string, markerId: number): string | null {
  if (!expr.trim()) return null;
  try {
    const mm = evaluateExpr(expr, { id: markerId });
    if (!Number.isFinite(mm)) return null;
    return `${mm.toFixed(1)} mm`;
  } catch {
    return null;
  }
}

/** Preview the result of evaluating a rotation expression (in degrees) */
function rotPreview(expr: string, markerId: number): string | null {
  if (!expr.trim()) return null;
  try {
    const deg = evaluateExpr(expr, { id: markerId });
    if (!Number.isFinite(deg)) return null;
    return `${deg.toFixed(1)}°`;
  } catch {
    return null;
  }
}

export function MarkerEditor({ marker, selectedIds }: Props) {
  const { t } = useTranslation();
  const ui = useMapStore((s) => s.markerUi[String(marker.id)]);
  const setExpr = useMapStore((s) => s.setExpr);
  const setVisible = useMapStore((s) => s.setVisible);
  const deselectAll = useMapStore((s) => s.deselectAll);

  const isMulti = selectedIds.length > 1;
  const canEditPose = marker.type === 'fixed' && marker.pose !== null;

  // Local inputs: label and size commit on Enter/blur; expressions commit on
  // Enter/blur for ALL selected markers.
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

  // Computed position / rotation for the primary marker (used for preview display)
  const primaryExprs = ui ?? {
    positionExpr: posExprLocal,
    rotationExpr: rotExprLocal,
  };
  const primaryPos = resolvePosition(primaryExprs.positionExpr, marker.id);
  const primaryRotEuler = quatToEulerDeg(resolveRotation(primaryExprs.rotationExpr, marker.id));

  const commitLabel = useCallback(() => {
    selectedIds.forEach((id) => void updateMarkerInfo(id, { markerFrameId: labelLocal }));
  }, [selectedIds, labelLocal]);

  const commitSize = useCallback(() => {
    const val = parseFloat(sizeLocal);
    if (!isNaN(val) && val > 0) {
      selectedIds.forEach((id) => void updateMarkerInfo(id, { sizeM: val / 1000 }));
    }
  }, [selectedIds, sizeLocal]);

  /** Apply position expression for a single axis to ALL selected markers, then persist. */
  const applyPosExpr = useCallback(
    (axis: 0 | 1 | 2) => {
      selectedIds.forEach((id) => {
        setExpr(id, axis, 'position', posExprLocal[axis]);
        void applyPose(id);
      });
    },
    [posExprLocal, selectedIds, setExpr],
  );

  /** Apply rotation expression for a single axis to ALL selected markers, then persist. */
  const applyRotExpr = useCallback(
    (axis: 0 | 1 | 2) => {
      selectedIds.forEach((id) => {
        setExpr(id, axis, 'rotation', rotExprLocal[axis]);
        void applyPose(id);
      });
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
      actions={<Badge tone={TYPE_TONES[marker.type]}>{t(TYPE_KEYS[marker.type])}</Badge>}
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
            {/* Position — always expressions */}
            <div>
              <div className="text-xs text-ink-muted mb-1">{t('map.positionExpr')}</div>
              <div className="flex gap-1">
                {(['X', 'Y', 'Z'] as const).map((axis, i) => {
                  const ax = i as 0 | 1 | 2;
                  const preview = posPreview(posExprLocal[ax], marker.id);
                  return (
                    <div key={axis} className="flex-1">
                      <div className="text-[10px] text-ink-faint text-center mb-0.5">
                        {axis} = {mToMm(primaryPos[ax]).toFixed(1)}
                      </div>
                      <input
                        type="text"
                        value={posExprLocal[ax]}
                        onChange={(e) =>
                          setPosExprLocal((prev) => {
                            const n: [string, string, string] = [...prev];
                            n[ax] = e.target.value;
                            return n;
                          })
                        }
                        onKeyDown={(e) => {
                          if (e.key === 'Enter') applyPosExpr(ax);
                        }}
                        onBlur={() => applyPosExpr(ax)}
                        placeholder={`${axis.toLowerCase()} expr`}
                        className={`${inputClasses} text-center`}
                      />
                      {preview && (
                        <div className="text-[10px] text-success mt-0.5 text-center">{preview}</div>
                      )}
                    </div>
                  );
                })}
              </div>
            </div>

            {/* Rotation — always expressions */}
            <div>
              <div className="text-xs text-ink-muted mb-1">{t('map.rotationExpr')}</div>
              <div className="flex gap-1">
                {(['X', 'Y', 'Z'] as const).map((axis, i) => {
                  const ax = i as 0 | 1 | 2;
                  const preview = rotPreview(rotExprLocal[ax], marker.id);
                  return (
                    <div key={axis} className="flex-1">
                      <div className="text-[10px] text-ink-faint text-center mb-0.5">
                        {axis} = {primaryRotEuler[ax].toFixed(1)}°
                      </div>
                      <input
                        type="text"
                        value={rotExprLocal[ax]}
                        onChange={(e) =>
                          setRotExprLocal((prev) => {
                            const n: [string, string, string] = [...prev];
                            n[ax] = e.target.value;
                            return n;
                          })
                        }
                        onKeyDown={(e) => {
                          if (e.key === 'Enter') applyRotExpr(ax);
                        }}
                        onBlur={() => applyRotExpr(ax)}
                        placeholder={`${axis.toLowerCase()} expr`}
                        className={`${inputClasses} text-center`}
                      />
                      {preview && (
                        <div className="text-[10px] text-success mt-0.5 text-center">{preview}</div>
                      )}
                    </div>
                  );
                })}
              </div>
            </div>
          </>
        )}

        {/* Visibility toggle — session only */}
        <label className="flex items-center gap-2 cursor-pointer text-xs text-ink">
          <input
            type="checkbox"
            checked={ui?.visible ?? true}
            onChange={(e) => selectedIds.forEach((id) => setVisible(id, e.target.checked))}
            className="accent-[var(--color-accent)]"
          />
          {t('map.visible')}
        </label>

        {/* Delete */}
        <Button variant="danger" className="w-full" onClick={handleDelete}>
          {isMulti ? t('map.deleteMany', { count: selectedIds.length }) : t('map.delete')}
        </Button>
      </div>
    </Panel>
  );
}
