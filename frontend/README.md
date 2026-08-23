# clover2 frontend

Web interface for the clover2 drone, built with Vite + React + TypeScript.

## Stack

- React 19, react-router 7, Tailwind 4, zustand 5, i18next (ru/en), lucide-react
- TypeScript strict with project references (`tsc -b`), oxlint
- IBM Plex Sans + JetBrains Mono bundled via `@fontsource` (offline-safe)
- Data comes from the clover2_http ROS2 backend (port 3000): REST discovery (`/api/manifest`, `/api/topics`, `/api/nodes`, `/api/node/info/-/{node}`, `/api/node/{publishers|subscribes|servers|clients}/-/{node}`) and one-way WebSocket topic streaming (`/ws/topic/json/-/{topic}`). The manifest-aware client lives in `src/api/`.

## Structure

```text
src/
  api/        layered clover2_http client (middleware chains + endpoints)
  types/      wire types shared with the backend
  constants/  backend/stream constants
  store/      zustand store with slices (manifest, topics, nodes, stream)
  hooks/      thin React hooks over the store and the API
  components/
    layout/   app chrome (Sidebar)
    ui/       design-system primitives (Button, Panel, ResourceState, ...)
    ros2/     ROS-domain components (rows, panels, gates, ListPage, MessageViewer)
  pages/
    ros2/     Nodes, NodeDetail, Topics, TopicDetail, Services
    ...       Dashboard, Settings
  i18n/       typed translations (en/ru)
```

## Scripts

- `npm run dev` — Vite dev server
- `npm run build` — `tsc -b && vite build`
- `npm run lint` — oxlint
- `npm run preview` — serve the production build
