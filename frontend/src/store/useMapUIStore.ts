import { create } from 'zustand';

export interface MapUIState {
  sidePanelOpen: boolean;
  showAxes: boolean;
  modalOpen: boolean;

  togglePanel: () => void;
  toggleAxes: () => void;
  setModalOpen: (open: boolean) => void;
}

export const useMapUIStore = create<MapUIState>()((set) => ({
  sidePanelOpen: true,
  showAxes: true,
  modalOpen: false,

  togglePanel: () => set((s) => ({ sidePanelOpen: !s.sidePanelOpen })),
  toggleAxes: () => set((s) => ({ showAxes: !s.showAxes })),
  setModalOpen: (open) => set({ modalOpen: open }),
}));
