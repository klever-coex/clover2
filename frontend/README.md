# clover2 frontend

Web interface for the clover2 drone, built with Vite + React + TypeScript.

## Stack

- React 19, react-router 7, Tailwind 4, zustand 5, i18next (ru/en), lucide-react
- TypeScript strict with project references (`tsc -b`), oxlint
- Data comes from the clover2_http ROS2 backend (port 8080): REST discovery (`/manifest`, `/topics`, `/nodes`, `/node/info/...`) and one-way WebSocket topic streaming (`/topic/json/-/{topic}`). The manifest-aware client lives in `src/api/clover2.ts`.

## Structure

```text
src/
  api/        manifest-aware clover2_http client
  types/      wire types shared with the backend
  constants/  backend/stream constants
  store/      zustand store with slices (manifest, topics, stream)
  hooks/      thin React hooks over the store
  components/ Sidebar, MessageViewer, per-page components
  pages/      Dashboard, Topics, TopicDetail, Settings
  i18n/       typed translations (en/ru)
```

## Scripts

- `npm run dev` — Vite dev server
- `npm run build` — `tsc -b && vite build`
- `npm run lint` — oxlint
- `npm run preview` — serve the production build
