import { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Loader2 } from 'lucide-react';
import { TopicRow } from '../components/Topics/TopicRow.tsx';
import { TopicSearch } from '../components/Topics/TopicSearch.tsx';
import { useRosStore } from '../store/useRosStore.ts';

/** Flat topic list with search; fetches the manifest and topic list from the backend. */
export function Topics() {
  const { t } = useTranslation();
  const manifest = useRosStore((s) => s.manifest);
  const manifestError = useRosStore((s) => s.manifestError);
  const hasCapability = useRosStore((s) => s.hasCapability);
  const fetchManifest = useRosStore((s) => s.fetchManifest);
  const topics = useRosStore((s) => s.topics);
  const topicsLoading = useRosStore((s) => s.topicsLoading);
  const topicsError = useRosStore((s) => s.topicsError);
  const reloadTopics = useRosStore((s) => s.reloadTopics);
  const [query, setQuery] = useState('');

  useEffect(() => {
    void fetchManifest();
  }, [fetchManifest]);

  useEffect(() => {
    if (manifest !== null && hasCapability('topics')) {
      void reloadTopics();
    }
  }, [manifest, hasCapability, reloadTopics]);

  const filtered = useMemo(() => {
    const q = query.trim().toLowerCase();
    if (q === '') return topics;
    return topics.filter(
      (topic) => topic.name.toLowerCase().includes(q) || topic.type.toLowerCase().includes(q),
    );
  }, [topics, query]);

  return (
    <div className="p-6">
      <h1 className="text-3xl font-bold mb-6">{t('topics.title')}</h1>

      {manifestError !== null && (
        <div className="flex flex-col items-center gap-4 p-8 bg-red-50 rounded-xl">
          <p className="text-red-700">{manifestError.message}</p>
          <button
            onClick={() => void fetchManifest()}
            className="px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition"
          >
            {t('topics.retry')}
          </button>
        </div>
      )}

      {manifestError === null && manifest !== null && !hasCapability('topics') && (
        <p className="text-gray-500">{t('topics.noCapability')}</p>
      )}

      {manifestError === null && manifest !== null && hasCapability('topics') && (
        <>
          <TopicSearch value={query} onChange={setQuery} />

          {topicsLoading && topics.length === 0 && (
            <div className="flex justify-center mt-8">
              <Loader2 className="animate-spin text-blue-500" size={32} />
            </div>
          )}

          {topicsError !== null && (
            <div className="flex flex-col items-center gap-4 mt-8 p-8 bg-red-50 rounded-xl">
              <p className="text-red-700">{topicsError.message}</p>
              <button
                onClick={() => void reloadTopics()}
                className="px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition"
              >
                {t('topics.retry')}
              </button>
            </div>
          )}

          {topicsError === null && !topicsLoading && filtered.length === 0 && (
            <p className="text-gray-400 mt-6">{t('topics.empty')}</p>
          )}

          {filtered.length > 0 && (
            <div className="mt-4 space-y-1">
              {filtered.map((topic) => (
                <TopicRow key={topic.name} topic={topic} />
              ))}
            </div>
          )}
        </>
      )}

      {manifest === null && manifestError === null && (
        <div className="flex justify-center mt-8">
          <Loader2 className="animate-spin text-blue-500" size={32} />
        </div>
      )}
    </div>
  );
}
