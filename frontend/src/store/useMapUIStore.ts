import { create } from 'zustand';

export interface MapUIState {
  sidePanelOpen: boolean;
  showAxes: boolean;

  togglePanel: () => void;
  toggleAxes: () => void;
}

export const useMapUIStore = create<MapUIState>()((set) => ({
  sidePanelOpen: true,
  showAxes: true,

  togglePanel: () => set((s) => ({ sidePanelOpen: !s.sidePanelOpen })),
  toggleAxes: () => set((s) => ({ showAxes: !s.showAxes })),
}));
