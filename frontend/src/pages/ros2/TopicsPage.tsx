import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { ListPage } from '../../components/ros2/ListPage.tsx';
import { SortSelect } from '../../components/ros2/SortSelect.tsx';
import { TopicRow } from '../../components/ros2/TopicRow.tsx';
import { useRosCapability } from '../../hooks/useRosCapability.ts';
import { useRosStore } from '../../store/useRosStore.ts';
import type { TopicInfo } from '../../types/topic.ts';

export function TopicsPage() {
  const { t } = useTranslation();
  const capability = useRosCapability('topics');
  const topics = useRosStore((s) => s.topics);
  const topicsLoading = useRosStore((s) => s.topicsLoading);
  const topicsError = useRosStore((s) => s.topicsError);
  const reloadTopics = useRosStore((s) => s.reloadTopics);
  const [sort, setSort] = useState<'name' | 'type'>('name');

  return (
    <ListPage<TopicInfo>
      title={t('topics.title')}
      capability={capability}
      noCapability={t('topics.noCapability')}
      searchPlaceholder={t('topics.searchPlaceholder')}
      emptyMessage={t('topics.empty')}
      items={topics}
      loading={topicsLoading}
      error={topicsError}
      onReload={reloadTopics}
      filter={(topic, q) =>
        topic.name.toLowerCase().includes(q) || topic.type.toLowerCase().includes(q)
      }
      keyOf={(topic) => topic.name}
      renderItem={(topic) => <TopicRow topic={topic} />}
      toolbarExtra={
        <SortSelect
          ariaLabel={t('common.sortBy')}
          value={sort}
          onChange={(value) => setSort(value as 'name' | 'type')}
          options={[
            { value: 'name', label: t('topics.sortName') },
            { value: 'type', label: t('topics.sortType') },
          ]}
        />
      }
      sortItems={(items) => {
        const sorted = [...items];
        sorted.sort((a, b) =>
          sort === 'name'
            ? a.name.localeCompare(b.name)
            : a.type.localeCompare(b.type) || a.name.localeCompare(b.name),
        );
        return sorted;
      }}
    />
  );
}
