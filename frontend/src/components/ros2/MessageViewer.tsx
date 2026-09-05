import { memo, useEffect, useRef, useState } from 'react';
import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';

import type { RosJsonValue } from '@/types/stream';
import { JsonCollapsible } from '../common/JsonCollapsible.tsx';
import { EmptyState } from '../common/EmptyState.tsx';

const MAX_ARRAY_ITEMS = 20;

interface RenderContext {
  objectCollapsed: Set<string>;
  arrayExpanded: Set<string>;
  toggleObject: (path: string) => void;
  toggleArray: (path: string) => void;
}

function renderRosValue(
  value: RosJsonValue,
  path: string,
  objectLabel: string,
  ctx: RenderContext,
): ReactNode {
  if (value === null) {
    return (
      <span key={path} className="text-json-null">
        null
      </span>
    );
  }

  if (typeof value !== 'object') {
    return (
      <span key={path} className="text-json-string">
        {String(value)}
      </span>
    );
  }

  if (Array.isArray(value)) {
    const open = ctx.arrayExpanded.has(path);
    return (
      <JsonCollapsible
        key={path}
        label={`[${value.length}]`}
        defaultOpen={false}
        open={open}
        onToggle={() => ctx.toggleArray(path)}
      >
        {open ? (
          <>
            {value
              .slice(0, MAX_ARRAY_ITEMS)
              .map((item, index) => renderRosValue(item, `${path}[${index}]`, objectLabel, ctx))}
            {value.length > MAX_ARRAY_ITEMS && (
              <div className="text-micro text-muted-foreground/80">
                … +{value.length - MAX_ARRAY_ITEMS}
              </div>
            )}
          </>
        ) : null}
      </JsonCollapsible>
    );
  }

  const entries = Object.entries(value);
  if (entries.length === 0) {
    return (
      <span key={path} className="text-muted-foreground/80">
        {'{}'}
      </span>
    );
  }

  const open = !ctx.objectCollapsed.has(path);
  return (
    <JsonCollapsible
      key={path}
      label={objectLabel}
      defaultOpen
      open={open}
      onToggle={() => ctx.toggleObject(path)}
    >
      {open
        ? entries.map(([field, fieldValue]) => (
            <div key={field} className="ml-4">
              <span className="text-json-key font-semibold">{field}:</span>{' '}
              {renderRosValue(fieldValue, `${path}.${field}`, objectLabel, ctx)}
            </div>
          ))
        : null}
    </JsonCollapsible>
  );
}

const MessageRow = memo(function MessageRow({
  message,
  objectLabel,
  ctx,
}: {
  message: RosJsonValue;
  objectLabel: string;
  ctx: RenderContext;
}) {
  return (
    <div className="border-b border-border py-2">
      {renderRosValue(message, 'root', objectLabel, ctx)}
    </div>
  );
});

const ANNOUNCE_INTERVAL_MS = 5000;

export function MessageViewer({ messages }: { messages: RosJsonValue[] }) {
  const { t } = useTranslation();
  const visible = messages.slice(-1);
  const [announced, setAnnounced] = useState(0);
  const lastAnnounceRef = useRef(0);
  const [objectCollapsed, setObjectCollapsed] = useState<Set<string>>(new Set());
  const [arrayExpanded, setArrayExpanded] = useState<Set<string>>(new Set());

  const ctx: RenderContext = {
    objectCollapsed,
    arrayExpanded,
    toggleObject: (path) =>
      setObjectCollapsed((prev) => {
        const next = new Set(prev);
        if (!next.delete(path)) next.add(path);
        return next;
      }),
    toggleArray: (path) =>
      setArrayExpanded((prev) => {
        const next = new Set(prev);
        if (!next.delete(path)) next.add(path);
        return next;
      }),
  };

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
    <div className="mt-6 overflow-x-auto rounded-panel border border-border bg-card font-mono text-foreground">
      <span className="sr-only" aria-live="polite">
        {announced > 0 && t('topicDetail.messageActivity', { count: announced })}
      </span>
      {visible.length === 0 && <EmptyState message={t('topicDetail.noMessages')} />}
      {visible.map((message, i) => (
        <MessageRow
          key={messages.length - 1 - i}
          message={message}
          objectLabel={objectLabel}
          ctx={ctx}
        />
      ))}
    </div>
  );
}
