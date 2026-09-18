import { useEffect } from 'react';

import { deleteMarkersWithConfirm, saveMap } from '../store/mapMutations.ts';
import { useConfirmStore } from '@/store/useConfirmStore';
import { useMapStore } from '@/store/useMapStore';
import { useMapUIStore } from '@/store/useMapUIStore';

const NUMERIC_EXPR = /^-?\d+(\.\d+)?$/;

export function useMapKeyboardShortcuts() {
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      const target = e.target as HTMLElement;
      if (
        target.tagName === 'INPUT' ||
        target.tagName === 'TEXTAREA' ||
        target.tagName === 'SELECT' ||
        target.isContentEditable
      ) {
        return;
      }

      if (useConfirmStore.getState().options !== null) return;

      const state = useMapStore.getState();
      const selIds = state.selectedMarkerIds;
      
      const isModifierPressed = e.ctrlKey || e.metaKey;
      const code = e.code;

      switch (code) {
        case 'Delete':
        case 'Backspace': {
          e.preventDefault();
          if (selIds.length === 0) break;

          void deleteMarkersWithConfirm([...selIds]);

          break;
        }

        case 'Escape':
          e.preventDefault();
          state.deselectAll();

          break;

        case 'KeyA':
          if (isModifierPressed) {
            e.preventDefault();
            state.selectAll();
          }

          break;

        case 'KeyG':
          e.preventDefault();
          useMapUIStore.getState().toggleAxes();

          break;

        case 'ArrowUp':
        case 'ArrowDown':
        case 'ArrowLeft':
        case 'ArrowRight': {
          if (selIds.length === 0) break;
          e.preventDefault();
          const stepMm = e.shiftKey ? 1000 : 100; // 1m with Shift, else 10cm

          let axis: 0 | 1 | 2 = 0;
          let sign = 0;
          if (code === 'ArrowUp') { axis = 1; sign = -1; }
          else if (code === 'ArrowDown') { axis = 1; sign = 1; }
          else if (code === 'ArrowLeft') { axis = 0; sign = -1; }
          else if (code === 'ArrowRight') { axis = 0; sign = 1; }

          selIds.forEach((id) => {
            const marker = state.markers[id];
            const ui = state.markerUi[id];
            if (!marker || !ui || marker.type !== 'fixed' || marker.pose === null) return;
            const expr = ui.positionExpr[axis].trim();
            if (!NUMERIC_EXPR.test(expr)) return;
            const newVal = parseFloat(expr) + sign * stepMm;
            state.setExpr(id, axis, 'position', newVal.toString());
          });

          break;
        }

        case 'KeyS':
          if (isModifierPressed) {
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
