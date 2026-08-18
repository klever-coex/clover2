import { useEffect, useMemo } from 'react';
import { useTranslation } from 'react-i18next';
import { Link, Navigate, useSearchParams } from 'react-router';
import { Loader2 } from 'lucide-react';
import { MessageViewer } from '../components/MessageViewer.tsx';
import { StatusBadge } from '../components/TopicDetail/StatusBadge.tsx';
import { useTopicStream } from '../hooks/useTopicStream.ts';
import { useRosStore } from '../store/useRosStore.ts';

/** Live stream of one topic; the topic name comes from the ?topic= query parameter. */
export function TopicDetail() {
  const { t } = useTranslation();
  const [searchParams] = useSearchParams();
  const topicName = searchParams.get('topic');
  const topics = useRosStore((s) => s.topics);
  const reloadTopics = useRosStore((s) => s.reloadTopics);

  const stream = useTopicStream(topicName);

  const topic = useMemo(
    () => topics.find((candidate) => candidate.name === topicName) ?? null,
    [topics, topicName],
  );

  // Deep links skip the list page; fetch the topic list once to resolve the type.
  useEffect(() => {
    if (topics.length === 0) {
      void reloadTopics();
    }
  }, [topics.length, reloadTopics]);

  if (topicName === null) {
    return <Navigate to="/topics" replace />;
  }

  const errorMessage =
    stream.error?.status === 1008 ? t('topicDetail.unknownTopic') : stream.error?.message;

  return (
    <div className="p-6">
      <div className="flex items-center justify-between gap-4 flex-wrap">
        <h1 className="text-3xl font-bold font-mono break-all">{topicName}</h1>
        <Link to="/topics" className="text-blue-400 hover:text-blue-500 font-medium transition">
          {t('topicDetail.backToTopics')}
        </Link>
      </div>

      <div className="flex items-center gap-3 mt-4 flex-wrap">
        <StatusBadge state={stream.state} />
        {topic !== null && (
          <span className="text-xs text-gray-500 bg-gray-200 px-2 py-0.5 rounded">{topic.type}</span>
        )}
        <span className="text-xs text-gray-400">
          {stream.received} {t('topicDetail.received')}
        </span>
        <div className="flex gap-2 ml-auto">
          <button
            onClick={stream.clear}
            className="px-3 py-1.5 text-sm border border-gray-300 rounded-lg hover:bg-gray-100 transition"
          >
            {t('topicDetail.clear')}
          </button>
          <button
            onClick={stream.retry}
            className="px-3 py-1.5 text-sm bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition"
          >
            {t('topics.retry')}
          </button>
        </div>
      </div>

      {stream.messages.length > 0 ? (
        <MessageViewer messages={stream.messages} />
      ) : (
        <div className="flex justify-center items-center h-[60vh]">
          <div className="flex flex-col items-center space-y-4">
            <Loader2 className={`animate-spin ${stream.state === 'error' ? 'text-red-400' : 'text-blue-500'}`} size={48} />
            {stream.state === 'error' && stream.error !== null ? (
              <>
                <span className="text-red-500">{errorMessage}</span>
                <button
                  onClick={stream.retry}
                  className="px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition"
                >
                  {t('topics.retry')}
                </button>
              </>
            ) : stream.state === 'closed' ? (
              <>
                <span className="text-gray-400">{t('topicDetail.closed')}</span>
                <button
                  onClick={stream.retry}
                  className="px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition"
                >
                  {t('topics.retry')}
                </button>
              </>
            ) : (
              <span className="text-gray-400">{t('topicDetail.noMessages')}</span>
            )}
          </div>
        </div>
      )}
    </div>
  );
}
