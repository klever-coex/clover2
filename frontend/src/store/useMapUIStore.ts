import { create } from 'zustand';

export interface MapUIState {
  sidePanelOpen: boolean;
  isTransforming: boolean;
  showAxes: boolean;

  togglePanel: () => void;
  setTransforming: (v: boolean) => void;
  toggleAxes: () => void;
}

export const useMapUIStore = create<MapUIState>()((set) => ({
  sidePanelOpen: true,
  isTransforming: false,
  showAxes: true,

  togglePanel: () => set((s) => ({ sidePanelOpen: !s.sidePanelOpen })),
  setTransforming: (v) => set({ isTransforming: v }),
  toggleAxes: () => set((s) => ({ showAxes: !s.showAxes })),
}));
