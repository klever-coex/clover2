import { useEffect } from 'react';

import i18n from '../i18n/index.ts';
import { mToMm } from '../constants/units.ts';
import { deleteMarkers, downloadProject, saveMap } from '../store/mapMutations.ts';
import { useMapStore } from '../store/useMapStore.ts';
import { useMapUIStore } from '../store/useMapUIStore.ts';
import { resolvePosition } from '../utils/transformUtils.ts';

export function useMapKeyboardShortcuts() {
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      const tag = (e.target as HTMLElement).tagName;
      if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;

      const state = useMapStore.getState();
      const selIds = state.selectedMarkerIds;

      switch (e.key) {
        case 'Delete':
        case 'Backspace': {
          e.preventDefault();
          if (selIds.length > 0 && confirm(i18n.t('map.deleteConfirmMany', { count: selIds.length }))) {
            const ids = [...selIds];
            state.deselectAll();
            requestAnimationFrame(() => void deleteMarkers(ids));
          }
          break;
        }

        case 'Escape':
          e.preventDefault();
          state.deselectAll();
          break;

        case 'a':
        case 'A':
          if (e.ctrlKey || e.metaKey) {
            e.preventDefault();
            state.selectAll();
          }
          break;

        case 'g':
        case 'G':
          e.preventDefault();
          useMapUIStore.getState().toggleAxes();
          break;

        case 'ArrowUp':
        case 'ArrowDown':
        case 'ArrowLeft':
        case 'ArrowRight': {
          if (selIds.length === 0) break;
          e.preventDefault();
          const stepMm = e.shiftKey ? 10 : 1; // 1cm or 1mm

          let axis: 0 | 1 | 2 = 0;
          let sign = 0;
          if (e.key === 'ArrowUp') { axis = 1; sign = -1; }
          else if (e.key === 'ArrowDown') { axis = 1; sign = 1; }
          else if (e.key === 'ArrowLeft') { axis = 0; sign = -1; }
          else if (e.key === 'ArrowRight') { axis = 0; sign = 1; }

          selIds.forEach((id) => {
            const marker = state.markers[id];
            const ui = state.markerUi[id];
            if (!marker || !ui || marker.type !== 'fixed' || marker.pose === null) return;
            const pos = resolvePosition(ui.positionExpr, marker.id);
            const posMm: [number, number, number] = [
              mToMm(pos[0]),
              mToMm(pos[1]),
              mToMm(pos[2]),
            ];
            const newVal = posMm[axis] + sign * stepMm;
            state.setExpr(id, axis, 'position', newVal.toString());
          });
          break;
        }

        case 's':
        case 'S':
          if ((e.ctrlKey || e.metaKey) && e.shiftKey) {
            e.preventDefault();
            downloadProject();
          } else if (e.ctrlKey || e.metaKey) {
            e.preventDefault();
            void saveMap();
          }
          break;
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);
}
