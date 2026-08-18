import { memo, useState } from 'react';
import type { ReactNode } from 'react';
import { RENDER_MESSAGE_CAP } from '../constants/ros.ts';
import type { RosJsonValue } from '../types/ros.ts';

function Collapsible({
  label,
  children,
  defaultOpen = true,
}: {
  label: string;
  children: ReactNode;
  defaultOpen?: boolean;
}) {
  const [isOpen, setIsOpen] = useState(defaultOpen);

  return (
    <div className="ml-4">
      <div
        className="flex items-center cursor-pointer select-none"
        onClick={() => setIsOpen(!isOpen)}
      >
        <span className="text-blue-400 font-semibold mr-1">
          {isOpen ? '▼' : '▶'} {label}:
        </span>
      </div>
      {isOpen && <div className="ml-4">{children}</div>}
    </div>
  );
}

/** Renders one ROS message value as collapsible sections; recursion is total over RosJsonValue. */
function renderRosValue(value: RosJsonValue, key: string): ReactNode {
  if (value === null) {
    return (
      <span key={key} className="text-red-400">
        null
      </span>
    );
  }
  if (typeof value !== 'object') {
    return (
      <span key={key} className="text-green-300">
        {String(value)}
      </span>
    );
  }
  if (Array.isArray(value)) {
    return (
      <Collapsible key={key} label={`[${value.length}]`} defaultOpen={false}>
        {value.map((item, index) => renderRosValue(item, String(index)))}
      </Collapsible>
    );
  }

  const entries = Object.entries(value);
  if (entries.length === 0) {
    return (
      <span key={key} className="text-gray-400">
        {'{}'}
      </span>
    );
  }
  return (
    <Collapsible key={key} label="object">
      {entries.map(([field, fieldValue]) => (
        <div key={field} className="ml-4">
          <span className="text-blue-400 font-semibold">{field}:</span>{' '}
          {renderRosValue(fieldValue, field)}
        </div>
      ))}
    </Collapsible>
  );
}

/** One streamed message; memoized so only new frames re-render. */
const MessageRow = memo(function MessageRow({ message }: { message: RosJsonValue }) {
  return (
    <div className="border-b border-gray-700 py-2">
      {renderRosValue(message, 'root')}
    </div>
  );
});

/** Rolling history of streamed messages, newest first. */
export function MessageViewer({ messages }: { messages: RosJsonValue[] }) {
  const visible = messages.slice(-RENDER_MESSAGE_CAP).reverse();

  return (
    <div className="p-4 bg-gray-800 rounded-lg text-white font-mono overflow-x-auto mt-6">
      {visible.map((message, i) => (
        <MessageRow key={messages.length - 1 - i} message={message} />
      ))}
    </div>
  );
}
