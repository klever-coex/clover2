import { memo, useEffect, useRef, useState } from 'react';
import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';

import type { RosJsonValue } from '../../types/stream.ts';
import { Collapsible } from '../ui/Collapsible.tsx';
import { EmptyState } from '../ui/EmptyState.tsx';

function renderRosValue(value: RosJsonValue, key: string, objectLabel: string): ReactNode {
  if (value === null) {
    return (
      <span key={key} className="text-json-null">
        null
      </span>
    );
  }

  if (typeof value !== 'object') {
    return (
      <span key={key} className="text-json-string">
        {String(value)}
      </span>
    );
  }

  if (Array.isArray(value)) {
    return (
      <Collapsible key={key} label={`[${value.length}]`} defaultOpen={false}>
        {value.map((item, index) => renderRosValue(item, String(index), objectLabel))}
      </Collapsible>
    );
  }

  const entries = Object.entries(value);
  if (entries.length === 0) {
    return (
      <span key={key} className="text-ink-faint">
        {'{}'}
      </span>
    );
  }

  return (
    <Collapsible key={key} label={objectLabel}>
      {entries.map(([field, fieldValue]) => (
        <div key={field} className="ml-4">
          <span className="text-json-key font-semibold">{field}:</span>{' '}
          {renderRosValue(fieldValue, field, objectLabel)}
        </div>
      ))}
    </Collapsible>
  );
}

const MessageRow = memo(function MessageRow({
  message,
  objectLabel,
}: {
  message: RosJsonValue;
  objectLabel: string;
}) {
  return (
    <div className="border-b border-line py-2">
      {renderRosValue(message, 'root', objectLabel)}
    </div>
  );
});

/** Announcement cadence for screen readers: first message immediately, then at most every 5s. */
const ANNOUNCE_INTERVAL_MS = 5000;

export function MessageViewer({ messages }: { messages: RosJsonValue[] }) {
  const { t } = useTranslation();
  const visible = messages.slice(-1);
  const [announced, setAnnounced] = useState(0);
  const lastAnnounceRef = useRef(0);

  useEffect(() => {
    if (messages.length === 0) return;
    const now = Date.now();
    if (now - lastAnnounceRef.current >= ANNOUNCE_INTERVAL_MS) {
      lastAnnounceRef.current = now;
      setAnnounced(messages.length);
    }
  }, [messages.length]);

  const objectLabel = t('topicDetail.object');

  return (
    <div className="bg-surface-1 border border-line rounded-panel text-ink font-mono overflow-x-auto mt-6">
      <span className="sr-only" aria-live="polite">
        {announced > 0 && t('topicDetail.messageActivity', { count: announced })}
      </span>
      {visible.length === 0 && <EmptyState message={t('topicDetail.noMessages')} />}
      {visible.map((message, i) => (
        <MessageRow key={messages.length - 1 - i} message={message} objectLabel={objectLabel} />
      ))}
    </div>
  );
}
