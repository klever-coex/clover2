import { useTranslation } from 'react-i18next';
import { ListPage } from '../../components/ros2/ListPage.tsx';
import { TopicRow } from '../../components/ros2/TopicRow.tsx';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useRosStore } from '../../store/useRosStore.ts';
import type { TopicInfo } from '../../types/topic.ts';

/** Flat topic list with search; gated on the backend 'topics' capability. */
export function TopicsPage() {
  const { t } = useTranslation();
  const capability = useRosCapability('topics');
  const topics = useRosStore((s) => s.topics);
  const topicsLoading = useRosStore((s) => s.topicsLoading);
  const topicsError = useRosStore((s) => s.topicsError);
  const reloadTopics = useRosStore((s) => s.reloadTopics);

  return (
    <ListPage<TopicInfo>
      title={t('topics.title')}
      capability={capability}
      noCapability={t('topics.noCapability')}
      searchPlaceholder={t('topics.searchPlaceholder')}
      emptyMessage={t('topics.empty')}
      items={topics}
      loading={topicsLoading}
      error={topicsError?.message ?? null}
      onReload={reloadTopics}
      filter={(topic, q) =>
        topic.name.toLowerCase().includes(q) || topic.type.toLowerCase().includes(q)
      }
      keyOf={(topic) => topic.name}
      renderItem={(topic) => <TopicRow topic={topic} />}
    />
  );
}
