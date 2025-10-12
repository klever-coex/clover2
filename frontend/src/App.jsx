import { Routes, Route, Link } from "react-router-dom";
import Sidebar from "./components/Sidebar";
import Dashboard from "./pages/Dashboard";
// import ROS2 from "./pages/ROS2";
// import TopicDetail from "./pages/TopicDetail";
import Settings from "./pages/Settings";

function App() {
  return (
    <div className="flex h-screen bg-gray-100">
      <Sidebar />
      <main className="flex-1 overflow-y-auto">
        <Routes>
          <Route path="/" element={<Dashboard />} />
          {/* <Route path="/ros2" element={<ROS2 />} /> */}
          <Route path="/settings" element={<Settings />} />
          {/* <Route path="/topics/-/:topicNameURL" element={<TopicDetail />} /> */}
        </Routes>
      </main>
    </div>
  );
}

export default App;
