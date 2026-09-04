import { lazy, Suspense } from 'react';
import { Route, Routes } from 'react-router';
import { useTranslation } from 'react-i18next';
import { Sidebar } from './components/layout/Sidebar.tsx';
import { ConfirmDialog } from './components/ui/ConfirmDialog.tsx';
import { LoadingState } from './components/ui/LoadingState.tsx';
import { useRosoutCollector } from './hooks/useRosoutCollector.ts';
import { Dashboard } from './pages/Dashboard.tsx';
import { Settings } from './pages/Settings.tsx';
import { LogsPage } from './pages/ros2/LogsPage.tsx';
import { NodeDetailPage } from './pages/ros2/NodeDetailPage.tsx';
import { NodesPage } from './pages/ros2/NodesPage.tsx';
import { TopicDetailPage } from './pages/ros2/TopicDetailPage.tsx';
import { TopicsPage } from './pages/ros2/TopicsPage.tsx';
import { VideoPage } from './pages/video/VideoPage.tsx';

const MapPage = lazy(() => import('./pages/map/MapPage.tsx'));

export default function App() {
  useRosoutCollector();
  const { t } = useTranslation();

  return (
    <div className="flex h-screen bg-surface-0 text-ink">
      <a
        href="#main-content"
        className="sr-only focus:not-sr-only focus:absolute focus:top-2 focus:left-2 focus:z-50 focus:rounded-row focus:bg-surface-2 focus:px-3 focus:py-2 focus:text-sm focus:text-ink"
      >
        {t('common.skipToContent')}
      </a>
      <Sidebar />
      <main id="main-content" className="flex-1 overflow-y-auto">
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route
            path="/map"
            element={
              <Suspense fallback={<LoadingState variant="centered" />}>
                <MapPage />
              </Suspense>
            }
          />
          <Route path="/video" element={<VideoPage />} />
          <Route path="/ros2/nodes" element={<NodesPage />} />
          <Route path="/ros2/nodes/detail" element={<NodeDetailPage />} />
          <Route path="/ros2/topics" element={<TopicsPage />} />
          <Route path="/ros2/topics/detail" element={<TopicDetailPage />} />
          <Route path="/ros2/logs" element={<LogsPage />} />
          <Route path="/settings" element={<Settings />} />
        </Routes>
      </main>
      <ConfirmDialog />
    </div>
  );
}
