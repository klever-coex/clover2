"use client";

import { useEffect, useMemo, useState } from "react";
import { useRouter } from "next/navigation";
import * as ROSLIB from "roslib";
import { ChevronDown, ChevronRight } from "lucide-react";

type Topic = {
  name: string;
  type: string;
};

type TopicTree = Record<
  string,
  {
    __children: TopicTree;
    __type: string | null;
  }
>;

type TreeNodeProps = {
  node: TopicTree;
  prefix?: string;
};

export default function Ros2Page() {
  const router = useRouter();
  const [topics, setTopics] = useState<Topic[]>([]);
  const [isConnected, setIsConnected] = useState(false);
  const [rosError, setRosError] = useState<string | null>(null);

  useEffect(() => {
    const ros = new ROSLIB.Ros({ encoding: "ascii" });
    const wsProtocol = window.location.protocol === "https:" ? "wss" : "ws";
    const wsUrl = `${wsProtocol}://${window.location.hostname}:9090`;
    ros.connect(wsUrl);

    ros.on("connection", () => {
      setIsConnected(true);
      setRosError(null);
      ros.getTopics((res: { topics: string[]; types: string[] }) => {
        const combined = res.topics.map((name, index) => ({
          name,
          type: res.types[index],
        }));
        setTopics(combined);
      });
    });

    ros.on("error", () => {
      setRosError(`ROS connection failed: ${wsUrl}`);
    });
    ros.on("close", () => {
      setIsConnected(false);
    });

    return () => {
      ros.close();
    };
  }, []);

  const buildTopicTree = (items: Topic[]) => {
    const tree: TopicTree = {};
    items.forEach(({ name, type }) => {
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

  const TreeNode = ({ node, prefix = "" }: TreeNodeProps) => {
    const [expanded, setExpanded] = useState(true);

    return (
      <ul className="ml-4 border-l border-gray-600 pl-3">
        {Object.entries(node).map(([key, value]) => {
          const hasChildren = Object.keys(value.__children).length > 0;
          const isTopic = Boolean(value.__type);
          const normalizedPrefix = prefix.startsWith("/")
            ? prefix.slice(1)
            : prefix;
          const nextPrefix = normalizedPrefix ? `${normalizedPrefix}/${key}` : key;

          return (
            <li key={key} className="my-1">
              <div className="flex items-center justify-between">
                <div className="flex items-center gap-1">
                  {hasChildren && (
                    <button
                      className="text-blue-400 font-semibold hover:text-blue-300"
                      onClick={() => setExpanded(!expanded)}
                      type="button"
                    >
                      {expanded ? <ChevronDown size={18} /> : <ChevronRight size={18} />}
                    </button>
                  )}
                  {isTopic ? (
                    <button
                      className="hover:text-blue-400"
                      onClick={() => router.push(`/topics-/${nextPrefix}`)}
                      type="button"
                    >
                      /{key}
                    </button>
                  ) : (
                    <span className="text-blue-400 font-semibold">{key}</span>
                  )}
                </div>

                {isTopic && (
                  <span className="text-gray-500 text-xs ml-2">
                    ({value.__type})
                  </span>
                )}
              </div>

              {hasChildren && expanded && (
                <TreeNode node={value.__children} prefix={`/${nextPrefix}`} />
              )}
            </li>
          );
        })}
      </ul>
    );
  };

  const tree = useMemo(() => buildTopicTree(topics), [topics]);

  return (
    <div className="p-6">
      <h1 className="text-3xl font-bold mb-2 text-black">ROS Topics</h1>
      <p className="text-gray-600 mb-6">Список доступных ROS-топиков и их типов.</p>

      {isConnected ? (
        <TreeNode node={tree} />
      ) : (
        <div className="space-y-2">
          <p className="text-gray-400">Connecting to ROS...</p>
          {rosError && <p className="text-red-400">{rosError}</p>}
        </div>
      )}
    </div>
  );
}
