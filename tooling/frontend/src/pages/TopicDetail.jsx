import ROSLIB from 'roslib';
import { useEffect, useState } from "react";
import { useParams, Link } from "react-router-dom";
import { ClipLoader } from "react-spinners";

const Collapsible = ({ label, children, defaultOpen = true }) => {
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

const renderMessage = (obj) => {
  if (obj === null) return <span className="text-red-400">null</span>;
  if (typeof obj === "undefined") return <span className="text-gray-400">undefined</span>;
  if (typeof obj !== "object") return <span className="text-green-300">{String(obj)}</span>;

  if (Array.isArray(obj)) {
    return obj.map((item, i) => (
      <Collapsible key={i} label={`[${i}]`} defaultOpen={false}>
        {renderMessage(item)}
      </Collapsible>
    ));
  }

  return Object.entries(obj).map(([key, value]) => {
    if (typeof value === "object" && value !== null && (Array.isArray(value) || Object.keys(value).length > 0)) {
      return (
        <Collapsible key={key} label={key}>
          {renderMessage(value)}
        </Collapsible>
      );
    } else {
      return (
        <div key={key} className="ml-4">
          <span className="text-blue-400 font-semibold">{key}:</span>{" "}
          {renderMessage(value)}
        </div>
      );
    }
  });
};

const TopicDetail = () => {
  const { topicNameURL } = useParams();
  let topicName = '/' + topicNameURL;
  const [message, setMessage] = useState(null);
  const [isConnected, setIsConnected] = useState(false);
  const [topicType, setTopicType] = useState(null);
  const [imageSrc, setImageSrc] = useState("http://127.0.0.1:8080/stream?topic=/image_raw");

  useEffect(() => {
    const ros = new ROSLIB.Ros({ encoding: "ascii" });

    ros.connect("ws://localhost:9090");

    ros.on("connection", () => {
      console.log("Connected to ROS!");
      setIsConnected(true);

      ros.getTopics((topicList) => {
        const index = topicList.topics.indexOf(decodeURIComponent(topicName));
        const type = topicList.types[index];
        setTopicType(type);

        if (!type) {
          console.error("Topic type not found!");
          return;
        }

        const subscriber = new ROSLIB.Topic({
          ros,
          name: decodeURIComponent(topicName),
          messageType: type,
        });

        if (type === "sensor_msgs/CompressedImage") {
          subscriber.subscribe((msg) => {
            setImageSrc(`data:image/${msg.format};base64,${msg.data}`);
          });
        } else if (type === "sensor_msgs/Image") {
          setImageSrc("http://127.0.0.1:8080/stream?topic=/image_raw");
          subscriber.subscribe((msg) => {
            setImageSrc(null);
            try {
              setImageSrc("http://127.0.0.1:8080/stream?topic=/image_raw");
            } catch (err) {
              console.error("Failed to render raw image:", err);
              setImageSrc(null);
            }
          });
        } else {
          subscriber.subscribe((msg) => {
            setMessage(msg);
          });
        }

        return () => {
          subscriber.unsubscribe();
          ros.close();
        };
      });
    });

    ros.on("error", (error) => console.error("ROS Error:", error));
    ros.on("close", () => setIsConnected(false));
  }, [topicName]);

  return (
    <div className="p-6 grid grid-flow-row-dense">
      <div className="flex justify-between items-center">
        <h1 className="text-3xl font-bold text-black">{decodeURIComponent(topicName)}</h1>
        <Link
          to="/ros2"
          className="ml-auto text-blue-400 hover:text-blue-500 font-medium transition"
        >
          Back to Topics
        </Link>
      </div>

      {isConnected ? (
        topicType?.includes("Image") ? (
          imageSrc ? (
            <div className="flex justify-center items-center mt-6">
              <img
                src="http://localhost:8080/stream?topic=/image_raw"
                alt="ROS Topic"
                className="rounded-lg shadow-lg max-h-[80vh] object-contain"
              />
            </div>
          ) : (
            <div className="flex justify-center items-center h-[60vh]">
              <div className="flex flex-col items-center space-y-4">
                <ClipLoader color="#3b82f6" loading={true} size={60} />
                <span className="text-gray-400">Waiting for image message...</span>
              </div>
            </div>
          )
        ) : !!message ? (
          <pre className="p-4 bg-gray-800 rounded-lg text-white font-mono overflow-x-auto mt-6">
            {renderMessage(message)}
          </pre>
        ) : (
          <div className="flex justify-center items-center h-[60vh]">
            <div className="flex flex-col items-center space-y-4">
              <ClipLoader color="#3b82f6" loading={true} size={60} />
              <span className="text-gray-400">Waiting for message...</span>
            </div>
          </div>
        )
      ) : (
        <div className="flex justify-center items-center h-[60vh]">
          <div className="flex flex-col items-center space-y-4">
            <ClipLoader color="#3b82f6" loading={true} size={60} />
            <span className="text-gray-400">Connecting to ROS...</span>
          </div>
        </div>
      )}
    </div>
  );
};

export default TopicDetail;
