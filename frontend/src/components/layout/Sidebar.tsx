import { useState } from 'react';
import { NavLink } from 'react-router';
import { useTranslation } from 'react-i18next';
import {
  BookOpenText,
  Cable,
  Camera,
  ChevronDown,
  ChevronUp,
  Drone,
  EthernetPort,
  Globe,
  Menu,
  Network,
  Orbit,
  Settings,
  SquarePen,
} from 'lucide-react';
import type { LucideIcon } from 'lucide-react';
import { cn } from '../../lib/cn.ts';
import type { TranslationKey } from '../../i18n/index.ts';
import { Button } from '../ui/Button.tsx';
import { IconButton } from '../ui/IconButton.tsx';
import { Select } from '../ui/Select.tsx';

type MenuItem =
  | { key: string; kind: 'route'; labelKey: TranslationKey; icon: LucideIcon; to: string }
  | { key: string; kind: 'external'; labelKey: TranslationKey; icon: LucideIcon; href: (lang: string) => string };

interface Ros2Item {
  key: string;
  labelKey: TranslationKey;
  icon: LucideIcon;
  to: string;
}

const { protocol, hostname } = window.location;

const PRIMARY_MENU: MenuItem[] = [
  { key: 'dashboard', kind: 'route', labelKey: 'sidebar.dashboard', icon: Drone, to: '/' },
  { key: 'settings', kind: 'route', labelKey: 'sidebar.settings', icon: Settings, to: '/settings' },
];

const ROS2_ITEMS: Ros2Item[] = [
  { key: 'nodes', labelKey: 'sidebar.nodes', icon: Network, to: '/ros2/nodes' },
  { key: 'topics', labelKey: 'sidebar.topics', icon: EthernetPort, to: '/ros2/topics' },
  { key: 'services', labelKey: 'sidebar.services', icon: Cable, to: '/ros2/services' },
];

const SECONDARY_MENU: MenuItem[] = [
  {
    key: 'documentation',
    kind: 'external',
    labelKey: 'sidebar.documentation',
    icon: BookOpenText,
    href: (lang) => `${protocol}//${hostname}:9000/${lang}/index.html`,
  },
  {
    key: 'camera',
    kind: 'external',
    labelKey: 'sidebar.camera',
    icon: Camera,
    href: () => `${protocol}//${hostname}:8081`,
  },
  {
    key: 'ide',
    kind: 'external',
    labelKey: 'sidebar.ide',
    icon: SquarePen,
    href: () => `${protocol}//${hostname}:9880/?folder=/home/pi`,
  },
];

const LANGUAGES = [
  { code: 'en', label: 'English' },
  { code: 'ru', label: 'Русский' },
] as const;

const itemClasses = (isOpen: boolean) => (isActive: boolean) =>
  cn(
    'flex items-center p-2 rounded-panel transition-colors duration-fast focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-royal/60',
    isOpen ? 'gap-3 justify-start' : 'justify-center',
    isActive
      ? 'bg-surface-2 text-accent-text shadow-[inset_2px_0_0_0_var(--color-accent)]'
      : 'text-ink-muted hover:bg-surface-2 hover:text-ink',
  );

export function Sidebar() {
  const [isOpen, setIsOpen] = useState(true);
  const [ros2Open, setRos2Open] = useState(true);
  const { t, i18n } = useTranslation();

  const changeLanguage = (lang: string) => {
    void i18n.changeLanguage(lang);
    localStorage.setItem('language', lang);
  };

  return (
    <div
      className={cn(
        'h-screen bg-surface-1 border-r border-line flex flex-col transition-all duration-slow',
        isOpen ? 'w-64' : 'w-20',
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
          onClick={() => setIsOpen(!isOpen)}
        />
      </div>

      {/* Menu */}
      <nav className="flex-1 p-4 space-y-1 overflow-y-auto">
        {PRIMARY_MENU.map((item) => {
          const Icon = item.icon;
          const to = item.kind === 'route' ? item.to : item.href(i18n.language);

          return (
            <NavLink
              key={item.key}
              to={to}
              target={item.kind === 'external' ? '_blank' : undefined}
              className={({ isActive }) => itemClasses(isOpen)(isActive)}
            >
              <Icon size={20} />
              <span className={isOpen ? undefined : 'sr-only'}>{t(item.labelKey)}</span>
            </NavLink>
          );
        })}

        {/* ROS2 group */}
        <Button
          variant="ghost"
          onClick={() => (isOpen ? setRos2Open(!ros2Open) : setIsOpen(true))}
          aria-expanded={isOpen && ros2Open}
          aria-controls="ros2-group"
          className={cn(
            'w-full rounded-panel',
            isOpen ? 'justify-start gap-3' : 'justify-center',
          )}
        >
          <Orbit size={20} />
          {isOpen ? (
            <>
              <span className="flex-1 text-left">{t('sidebar.ros2')}</span>
              {ros2Open ? <ChevronUp size={16} /> : <ChevronDown size={16} />}
            </>
          ) : (
            <span className="sr-only">{t('sidebar.ros2')}</span>
          )}
        </Button>

        {isOpen && ros2Open && (
          <div id="ros2-group" className="space-y-1">
            {ROS2_ITEMS.map((item) => {
              const Icon = item.icon;

              return (
                <NavLink
                  key={item.key}
                  to={item.to}
                  className={({ isActive }) => cn(itemClasses(isOpen)(isActive), 'ml-4')}
                >
                  <Icon size={20} />
                  <span>{t(item.labelKey)}</span>
                </NavLink>
              );
            })}
          </div>
        )}

        {SECONDARY_MENU.map((item) => {
          const Icon = item.icon;
          const to = item.kind === 'route' ? item.to : item.href(i18n.language);

          return (
            <NavLink
              key={item.key}
              to={to}
              target={item.kind === 'external' ? '_blank' : undefined}
              className={({ isActive }) => itemClasses(isOpen)(isActive)}
            >
              <Icon size={20} />
              <span className={isOpen ? undefined : 'sr-only'}>{t(item.labelKey)}</span>
            </NavLink>
          );
        })}
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
            onChange={(e) => changeLanguage(e.target.value)}
          >
            {LANGUAGES.map((lang) => (
              <option key={lang.code} value={lang.code}>
                {lang.label}
              </option>
            ))}
          </Select>
        )}
      </div>
    </div>
  );
}
