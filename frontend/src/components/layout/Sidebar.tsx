import { useEffect, useState } from 'react';
import { createPortal } from 'react-dom';
import { NavLink } from 'react-router';
import { useTranslation } from 'react-i18next';
import {
  BookOpenText,
  Camera,
  ChevronDown,
  Drone,
  EthernetPort,
  Globe,
  Map,
  Menu,
  Network,
  Orbit,
  Settings,
  SquarePen,
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
  /** Internal route path or an external-URL builder. */
  to: string | ((lang: string) => string);
}

interface NavGroupItem {
  key: string;
  labelKey: TranslationKey;
  icon: LucideIcon;
  children: NavLinkItem[];
}

type NavItem = NavLinkItem | NavGroupItem;

const { protocol, hostname } = window.location;

/** Single unified navigation tree — groups nest their children without any offset. */
const MENU: NavItem[] = [
  { key: 'dashboard', labelKey: 'sidebar.dashboard', icon: Drone, to: '/' },
  { key: 'map', labelKey: 'sidebar.map', icon: Map, to: '/map' },
  { key: 'settings', labelKey: 'sidebar.settings', icon: Settings, to: '/settings' },
  {
    key: 'ros2',
    labelKey: 'sidebar.ros2',
    icon: Orbit,
    children: [
      { key: 'nodes', labelKey: 'sidebar.nodes', icon: Network, to: '/ros2/nodes' },
      { key: 'topics', labelKey: 'sidebar.topics', icon: EthernetPort, to: '/ros2/topics' },
    ],
  },
  {
    key: 'documentation',
    labelKey: 'sidebar.documentation',
    icon: BookOpenText,
    to: (lang) => `${protocol}//${hostname}:9000/${lang}/index.html`,
  },
  {
    key: 'camera',
    labelKey: 'sidebar.camera',
    icon: Camera,
    to: () => `${protocol}//${hostname}:8081`,
  },
  {
    key: 'ide',
    labelKey: 'sidebar.ide',
    icon: SquarePen,
    to: () => `${protocol}//${hostname}:9880/?folder=/home/pi`,
  },
];

const LANGUAGES = [
  { code: 'en', label: 'English' },
  { code: 'ru', label: 'Русский' },
] as const;

/** Shared row style for every nav item — links, groups, and their children. */
const rowBase =
  'flex items-center p-2 rounded-panel transition-colors duration-fast focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-focus/60';

export function Sidebar() {
  const [isOpen, setIsOpen] = useState(true);
  const [openGroups, setOpenGroups] = useState<Set<string>>(() => new Set(['ros2']));
  const [flyoutKey, setFlyoutKey] = useState<string | null>(null);
  const [flyoutRect, setFlyoutRect] = useState<DOMRect | null>(null);
  const { t, i18n } = useTranslation();

  const changeLanguage = (lang: string) => {
    void i18n.changeLanguage(lang);
    localStorage.setItem('language', lang);
  };

  // Close the collapsed-sidebar group flyout on outside click or Escape.
  useEffect(() => {
    if (flyoutKey === null) return;

    const onMouseDown = (event: MouseEvent) => {
      const target = event.target;
      if (!(target instanceof Element) || !target.closest('[data-nav-flyout]')) {
        setFlyoutKey(null);
      }
    };
    const onKeyDown = (event: KeyboardEvent) => {
      if (event.key === 'Escape') setFlyoutKey(null);
    };

    document.addEventListener('mousedown', onMouseDown);
    document.addEventListener('keydown', onKeyDown);
    return () => {
      document.removeEventListener('mousedown', onMouseDown);
      document.removeEventListener('keydown', onKeyDown);
    };
  }, [flyoutKey]);

  const itemClasses = (active: boolean, showLabel: boolean) =>
    cn(
      rowBase,
      showLabel ? 'gap-3 justify-start' : 'justify-center',
      active
        ? 'bg-surface-2 text-accent-text shadow-[inset_2px_0_0_0_var(--color-accent)]'
        : 'text-ink-muted hover:bg-surface-2 hover:text-ink',
    );

  const isGroupOpen = (key: string) => openGroups.has(key);

  const toggleGroup = (key: string, rect?: DOMRect) => {
    // Collapsed sidebar: toggle a flyout with the group children — no expansion.
    if (!isOpen) {
      const nextOpen = flyoutKey !== key;
      setFlyoutKey(nextOpen ? key : null);
      if (nextOpen && rect !== undefined) setFlyoutRect(rect);
      return;
    }
    setFlyoutKey(null);
    setOpenGroups((prev) => {
      const next = new Set(prev);
      if (next.has(key)) next.delete(key);
      else next.add(key);
      return next;
    });
  };

  const renderLink = (item: NavLinkItem, showLabel = isOpen) => {
    const Icon = item.icon;
    const label = (
      <span className={showLabel ? 'whitespace-nowrap' : 'sr-only'}>{t(item.labelKey)}</span>
    );

    if (typeof item.to === 'string') {
      return (
        <NavLink
          key={item.key}
          to={item.to}
          onClick={() => setFlyoutKey(null)}
          className={({ isActive }) => itemClasses(isActive, showLabel)}
        >
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
        onClick={() => setFlyoutKey(null)}
        className={itemClasses(false, showLabel)}
      >
        <Icon size={20} className="shrink-0" />
        {label}
      </a>
    );
  };

  const renderItem = (item: NavItem) => {
    if ('children' in item) {
      const expanded = isGroupOpen(item.key);
      const flyoutOpen = flyoutKey === item.key;
      const Icon = item.icon;

      return (
        <div key={item.key} data-nav-flyout>
          <button
            type="button"
            onClick={(event) => toggleGroup(item.key, event.currentTarget.getBoundingClientRect())}
            aria-expanded={isOpen ? expanded : flyoutOpen}
            aria-controls={isOpen ? `nav-group-${item.key}` : `nav-flyout-${item.key}`}
            className={cn(
              rowBase,
              'w-full',
              isOpen ? 'gap-3 justify-start' : 'justify-center',
              isOpen && expanded ? 'text-ink' : 'text-ink-muted',
              'hover:bg-surface-2 hover:text-ink',
            )}
          >
            <Icon size={20} className="shrink-0" />
            {isOpen ? (
              <>
                <span className="flex-1 text-left whitespace-nowrap">{t(item.labelKey)}</span>
                <ChevronDown
                  size={16}
                  className={cn(
                    'shrink-0 transition-transform duration-normal',
                    expanded && 'rotate-180',
                  )}
                />
              </>
            ) : (
              <span className="sr-only">{t(item.labelKey)}</span>
            )}
          </button>

          {isOpen ? (
            <div
              id={`nav-group-${item.key}`}
              className={cn(
                'grid transition-[grid-template-rows] duration-normal',
                expanded ? 'grid-rows-[1fr]' : 'grid-rows-[0fr]',
              )}
            >
              <div
                className={cn(
                  'overflow-hidden min-h-0 space-y-1',
                  expanded ? 'visible' : 'invisible',
                )}
              >
                {item.children.map((child) => renderLink(child))}
              </div>
            </div>
          ) : (
            flyoutOpen &&
            flyoutRect !== null &&
            createPortal(
              <div
                id={`nav-flyout-${item.key}`}
                data-nav-flyout
                className="fixed z-50 w-56 rounded-panel border border-line bg-surface-1 p-1 space-y-1"
                style={{ left: flyoutRect.right + 8, top: flyoutRect.top }}
              >
                {item.children.map((child) => renderLink(child, true))}
              </div>,
              document.body,
            )
          )}
        </div>
      );
    }

    return renderLink(item);
  };

  return (
    <div
      className={cn(
        'h-screen bg-surface-1 border-r border-line flex flex-col transition-all duration-slow',
        // Clip content only while expanding — the group flyout lives outside
        // the sidebar (portal), so a closed sidebar must not clip horizontally.
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
          onClick={() => {
            setIsOpen((current) => !current);
            setFlyoutKey(null);
          }}
        />
      </div>

      {/* Menu */}
      <nav className="flex-1 p-4 space-y-1 overflow-y-auto">
        {MENU.map(renderItem)}
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
