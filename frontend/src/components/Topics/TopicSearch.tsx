import { useTranslation } from 'react-i18next';

export function TopicSearch({
  value,
  onChange,
}: {
  value: string;
  onChange: (value: string) => void;
}) {
  const { t } = useTranslation();

  return (
    <input
      type="text"
      value={value}
      onChange={(e) => onChange(e.target.value)}
      placeholder={t('topics.searchPlaceholder')}
      className="w-full p-2 border border-gray-300 rounded-lg text-sm outline-none focus:border-blue-400 transition"
    />
  );
}
