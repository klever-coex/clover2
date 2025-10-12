import ROSLIB from 'roslib';
import { useState, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import { ChevronDown, ChevronRight } from "lucide-react";

const ROS2 = () => {
  const [topics, setTopics] = useState([]);
  const [isConnected, setIsConnected] = useState(false);
  const navigate = useNavigate();

  useEffect(() => {
    const ros = new ROSLIB.Ros({ encoding: "ascii" });
    console.log(window.location.hostname);
    ros.connect("ws://" + window.location.hostname + ":9090");

    ros.on("connection", () => {
      console.log("Connected to ROS!");
      setIsConnected(true);

      ros.getTopics((res) => {
        const combined = res.topics.map((name, i) => ({
          name,
          type: res.types[i],
        }));
        setTopics(combined);
      });
    });

    ros.on("error", (error) => console.error("ROS Error:", error));
    ros.on("close", () => {
      console.log("Connection closed");
      setIsConnected(false);
    });

    return () => ros.close();
  }, []);

  const buildTopicTree = (topics) => {
    const tree = {};
    topics.forEach(({ name, type }) => {
      const parts = name.split("/").filter(Boolean);
      let current = tree;
      parts.forEach((part, index) => {
        if (!current[part]) current[part] = { __children: {}, __type: null };
        if (index === parts.length - 1) {
          current[part].__type = type;
        }
        current = current[part].__children;
      });
    });
    return tree;
  };

  const TreeNode = ({ node, prefix = "" }) => {
  const [expanded, setExpanded] = useState(true);

  return (
    <ul className="ml-4 border-l border-gray-600 pl-3">
      {Object.entries(node).map(([key, value]) => {
        const hasChildren = Object.keys(value.__children).length > 0;
        const isTopic = !!value.__type;

        return (
          <li key={key} className="my-1">
            <div className="flex items-center justify-between">
              <div className="flex items-center gap-1">
                {hasChildren && (
                  <button
                    className="text-blue-400 font-semibold hover:text-blue-300"
                    onClick={() => setExpanded(!expanded)}
                  >
                    {expanded ? <ChevronDown size={18} /> : <ChevronRight size={18} />}
                  </button>
                )}
                {isTopic ? (
                  <button
                    className="hover:text-blue-400"
                    onClick={() => navigate(`/topics/-${prefix}/${key}`)}
                  >
                    /{key}
                  </button>
                ) : (
                  <span className="text-blue-400 font-semibold">{key}</span>
                )}
              </div>

              {isTopic && (
                <span className="text-gray-500 text-xs ml-2">({value.__type})</span>
              )}
            </div>

            {hasChildren && expanded && (
              <TreeNode node={value.__children} prefix={`${prefix}/${key}`} />
            )}
          </li>
        );
      })}
    </ul>
  );
};

  const tree = buildTopicTree(topics);

  return (
    <div className="p-6">
      <h1 className="text-3xl font-bold mb-6">ROS Topics</h1>

      {isConnected ? (
        <TreeNode node={tree} />
      ) : (
        <p className="text-gray-400">Connecting to ROS...</p>
      )}
    </div>
  );
};

export default ROS2;
