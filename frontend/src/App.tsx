import { Route, Routes } from 'react-router';
import { Sidebar } from './components/layout/Sidebar.tsx';
import { Dashboard } from './pages/Dashboard.tsx';
import { Settings } from './pages/Settings.tsx';
import { NodeDetailPage } from './pages/ros2/NodeDetailPage.tsx';
import { NodesPage } from './pages/ros2/NodesPage.tsx';
import { ServicesPage } from './pages/ros2/ServicesPage.tsx';
import { TopicDetailPage } from './pages/ros2/TopicDetailPage.tsx';
import { TopicsPage } from './pages/ros2/TopicsPage.tsx';

export default function App() {
  return (
    <div className="flex h-screen bg-surface-0 text-ink">
      <Sidebar />
      <main className="flex-1 overflow-y-auto">
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/ros2/nodes" element={<NodesPage />} />
          <Route path="/ros2/nodes/detail" element={<NodeDetailPage />} />
          <Route path="/ros2/topics" element={<TopicsPage />} />
          <Route path="/ros2/topics/detail" element={<TopicDetailPage />} />
          <Route path="/ros2/services" element={<ServicesPage />} />
          <Route path="/settings" element={<Settings />} />
        </Routes>
      </main>
    </div>
  );
}
