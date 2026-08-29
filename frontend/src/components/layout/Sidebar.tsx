import { useState } from 'react';
import { NavLink, useLocation } from 'react-router';
import { useTranslation } from 'react-i18next';
import {
  BookOpenText,
  EthernetPort,
  Globe,
  Map,
  Menu,
  Network,
  ScrollText,
  Settings,
  SquarePen,
  Video,
} from 'lucide-react';
import type { LucideIcon } from 'lucide-react';
import { cn } from '../../lib/cn.ts';
import type { TranslationKey } from '../../i18n/index.ts';
import { IconButton } from '../ui/IconButton.tsx';
import { Select } from '../ui/Select.tsx';

interface NavLinkItem {
  key: string;
  labelKey: TranslationKey;
  icon: LucideIcon;
  to: string | ((lang: string) => string);
}

interface NavSection {
  key: string;
  labelKey?: TranslationKey;
  items: NavLinkItem[];
}

const { protocol, hostname } = window.location;

const MENU: NavSection[] = [
  {
    key: 'main',
    items: [
      { key: 'map', labelKey: 'sidebar.map', icon: Map, to: '/map' },
      { key: 'video', labelKey: 'sidebar.video', icon: Video, to: '/video' },
      { key: 'settings', labelKey: 'sidebar.settings', icon: Settings, to: '/settings' },
    ],
  },
  {
    key: 'ros2',
    labelKey: 'sidebar.ros2',
    items: [
      { key: 'nodes', labelKey: 'sidebar.nodes', icon: Network, to: '/ros2/nodes' },
      { key: 'topics', labelKey: 'sidebar.topics', icon: EthernetPort, to: '/ros2/topics' },
      { key: 'logs', labelKey: 'sidebar.logs', icon: ScrollText, to: '/ros2/logs' },
    ],
  },
  {
    key: 'external',
    items: [
      {
        key: 'documentation',
        labelKey: 'sidebar.documentation',
        icon: BookOpenText,
        to: (lang) => `${protocol}//${hostname}:9000/${lang}/index.html`,
      },
      {
        key: 'ide',
        labelKey: 'sidebar.ide',
        icon: SquarePen,
        to: () => `${protocol}//${hostname}:9880/?folder=/home/pi`,
      },
    ],
  },
];

const LANGUAGES = [
  { code: 'en', label: 'English' },
  { code: 'ru', label: 'Русский' },
] as const;

const rowBase =
  'flex items-center p-2 rounded-panel transition-colors duration-fast focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60';

export function Sidebar() {
  const [isOpen, setIsOpen] = useState(true);
  const { t, i18n } = useTranslation();
  const { pathname } = useLocation();

  const changeLanguage = (lang: string) => {
    void i18n.changeLanguage(lang);
    localStorage.setItem('language', lang);
  };

  const itemClasses = (active: boolean) =>
    cn(
      rowBase,
      isOpen ? 'gap-3 justify-start' : 'justify-center',
      active
        ? 'bg-surface-2 text-accent-text shadow-[inset_2px_0_0_0_var(--color-accent)]'
        : 'text-ink-muted hover:bg-surface-2 hover:text-ink',
    );

  const isItemActive = (item: NavLinkItem) =>
    typeof item.to === 'string' &&
    (pathname === item.to || pathname.startsWith(`${item.to}/`));

  const renderLink = (item: NavLinkItem) => {
    const Icon = item.icon;
    const label = (
      <span className={isOpen ? 'whitespace-nowrap' : 'sr-only'}>{t(item.labelKey)}</span>
    );

    if (typeof item.to === 'string') {
      return (
        <NavLink key={item.key} to={item.to} className={() => itemClasses(isItemActive(item))}>
          <Icon size={20} className="shrink-0" />
          {label}
        </NavLink>
      );
    }

    return (
      <a
        key={item.key}
        href={item.to(i18n.language)}
        target="_blank"
        rel="noreferrer"
        className={itemClasses(false)}
      >
        <Icon size={20} className="shrink-0" />
        {label}
      </a>
    );
  };

  return (
    <div
      className={cn(
        'h-screen bg-surface-1 border-r border-line flex flex-col transition-all duration-slow',
        isOpen ? 'w-64 overflow-hidden' : 'w-20',
      )}
    >
      {/* Header */}
      <div
        className={cn(
          'flex items-center p-4 border-b border-line',
          isOpen ? 'justify-between' : 'justify-center',
        )}
      >
        {isOpen && <span className="text-2xl font-bold">{t('header.title')}</span>}
        <IconButton
          icon={Menu}
          label={t('sidebar.toggleMenu')}
          expanded={isOpen}
          onClick={() => setIsOpen((current) => !current)}
        />
      </div>

      {/* Menu */}
      <nav className="flex-1 p-4 space-y-4 overflow-y-auto">
        {MENU.map((section, index) => (
          <div key={section.key}>
            {section.labelKey !== undefined && isOpen ? (
              <div
                className={cn(
                  'flex items-center gap-2 px-2 pb-1 text-[10px] font-semibold uppercase tracking-widest',
                  section.items.some(isItemActive) ? 'text-ink' : 'text-ink-faint',
                )}
              >
                {t(section.labelKey)}
                <span className="h-px flex-1 bg-line/40" />
              </div>
            ) : (
              index > 0 && <div className="mx-2 mb-2 h-px bg-line/40" />
            )}
            <div className="space-y-1">{section.items.map(renderLink)}</div>
          </div>
        ))}
      </nav>

      {/* Language Selector */}
      <div
        className={cn(
          'border-t border-line p-4',
          isOpen ? 'flex items-center justify-between' : 'flex justify-center',
        )}
      >
        <Globe size={20} className="text-ink-muted" />
        {isOpen && (
          <Select
            aria-label={t('sidebar.language')}
            value={i18n.language}
            onChange={(lang) => changeLanguage(lang)}
            options={LANGUAGES.map((lang) => ({ value: lang.code, label: lang.label }))}
            direction="up"
          />
        )}
      </div>
    </div>
  );
}
