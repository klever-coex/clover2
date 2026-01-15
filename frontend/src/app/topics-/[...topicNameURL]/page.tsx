"use client";

import { useEffect, useMemo, useState } from "react";
import type { ReactNode } from "react";
import Link from "next/link";
import { useParams } from "next/navigation";
import * as ROSLIB from "roslib";
import { ClipLoader } from "react-spinners";

type CollapsibleProps = {
  label: string;
  children: ReactNode;
  defaultOpen?: boolean;
};

const Collapsible = ({ label, children, defaultOpen = true }: CollapsibleProps) => {
  const [isOpen, setIsOpen] = useState(defaultOpen);

  return (
    <div className="ml-4">
      <div
        className="flex items-center cursor-pointer select-none"
        onClick={() => setIsOpen(!isOpen)}
      >
        <span className="text-blue-400 font-semibold mr-1">
          {isOpen ? "▼" : "▶"} {label}:
        </span>
      </div>
      {isOpen && <div className="ml-4">{children}</div>}
    </div>
  );
};

const renderMessage = (obj: unknown): React.ReactNode => {
  if (obj === null) return <span className="text-red-400">null</span>;
  if (typeof obj === "undefined") {
    return <span className="text-gray-400">undefined</span>;
  }
  if (typeof obj !== "object") {
    return <span className="text-green-300">{String(obj)}</span>;
  }

  if (Array.isArray(obj)) {
    return obj.map((item, index) => (
      <Collapsible key={index} label={`[${index}]`} defaultOpen={false}>
        {renderMessage(item)}
      </Collapsible>
    ));
  }

  return Object.entries(obj as Record<string, unknown>).map(([key, value]) => {
    if (
      typeof value === "object" &&
      value !== null &&
      (Array.isArray(value) || Object.keys(value).length > 0)
    ) {
      return (
        <Collapsible key={key} label={key}>
          {renderMessage(value)}
        </Collapsible>
      );
    }
    return (
      <div key={key} className="ml-4">
        <span className="text-blue-400 font-semibold">{key}:</span>{" "}
        {renderMessage(value)}
      </div>
    );
  });
};

export default function TopicDetailPage() {
  const params = useParams<{ topicNameURL?: string[] }>();
  const topicPath = useMemo(() => {
    if (!params.topicNameURL) return "";
    return Array.isArray(params.topicNameURL)
      ? params.topicNameURL.join("/")
      : params.topicNameURL;
  }, [params.topicNameURL]);

  const topicName = useMemo(
    () => `/${decodeURIComponent(topicPath)}`,
    [topicPath],
  );

  const [message, setMessage] = useState<unknown>(null);
  const [isConnected, setIsConnected] = useState(false);
  const [topicType, setTopicType] = useState<string | null>(null);
  const [rosError, setRosError] = useState<string | null>(null);
  const [imageSrc, setImageSrc] = useState<string | null>(
    "http://127.0.0.1:8080/stream?topic=/image_raw",
  );

  useEffect(() => {
    const ros = new ROSLIB.Ros({ encoding: "ascii" });
    const wsProtocol = window.location.protocol === "https:" ? "wss" : "ws";
    const wsUrl = `${wsProtocol}://${window.location.hostname}:9090`;
    ros.connect(wsUrl);

    ros.on("connection", () => {
      setIsConnected(true);
      setRosError(null);

      ros.getTopics((topicList: { topics: string[]; types: string[] }) => {
        const index = topicList.topics.indexOf(decodeURIComponent(topicName));
        const type = topicList.types[index];
        setTopicType(type);

        if (!type) {
          // eslint-disable-next-line no-console
          console.error("Topic type not found!");
          return;
        }

        const subscriber = new ROSLIB.Topic({
          ros,
          name: decodeURIComponent(topicName),
          messageType: type,
        });

        if (type === "sensor_msgs/CompressedImage") {
          subscriber.subscribe((msg: { format: string; data: string }) => {
            setImageSrc(`data:image/${msg.format};base64,${msg.data}`);
          });
        } else if (type === "sensor_msgs/Image") {
          setImageSrc("http://127.0.0.1:8080/stream?topic=/image_raw");
          subscriber.subscribe(() => {
            setImageSrc(null);
            try {
              setImageSrc("http://127.0.0.1:8080/stream?topic=/image_raw");
            } catch (err) {
              // eslint-disable-next-line no-console
              console.error("Failed to render raw image:", err);
              setImageSrc(null);
            }
          });
        } else {
          subscriber.subscribe((msg: unknown) => {
            setMessage(msg);
          });
        }

        return () => {
          subscriber.unsubscribe();
          ros.close();
        };
      });
    });

    ros.on("error", () => {
      setRosError(`ROS connection failed: ${wsUrl}`);
    });
    ros.on("close", () => setIsConnected(false));
  }, [topicName]);

  return (
    <div className="p-6 grid grid-flow-row-dense">
      <div className="flex justify-between items-center">
        <h1 className="text-3xl font-bold text-black">
          {decodeURIComponent(topicName)}
        </h1>
        <Link
          href="/ros2"
          className="ml-auto text-blue-400 hover:text-blue-500 font-medium transition"
        >
          Back to Topics
        </Link>
      </div>
      <p className="text-gray-600 mt-2">Просмотр данных выбранного ROS-топика.</p>

      {isConnected ? (
        topicType?.includes("Image") ? (
          imageSrc ? (
            <div className="flex justify-center items-center mt-6">
              <img
                src={imageSrc}
                alt="ROS Topic"
                className="rounded-lg shadow-lg max-h-[80vh] object-contain"
              />
            </div>
          ) : (
            <div className="flex justify-center items-center h-[60vh]">
              <div className="flex flex-col items-center space-y-4">
                <ClipLoader color="#3b82f6" loading size={60} />
                <span className="text-gray-400">Waiting for image message...</span>
              </div>
            </div>
          )
        ) : message ? (
          <pre className="p-4 bg-gray-800 rounded-lg text-white font-mono overflow-x-auto mt-6">
            {renderMessage(message)}
          </pre>
        ) : (
          <div className="flex justify-center items-center h-[60vh]">
            <div className="flex flex-col items-center space-y-4">
              <ClipLoader color="#3b82f6" loading size={60} />
              <span className="text-gray-400">Waiting for message...</span>
            </div>
          </div>
        )
      ) : (
        <div className="flex justify-center items-center h-[60vh]">
          <div className="flex flex-col items-center space-y-4">
            <ClipLoader color="#3b82f6" loading size={60} />
            <span className="text-gray-400">Connecting to ROS...</span>
            {rosError && <span className="text-red-400">{rosError}</span>}
          </div>
        </div>
      )}
    </div>
  );
}
