import { useState } from 'react';
import { NavLink } from 'react-router';
import { useTranslation } from 'react-i18next';
import { BookOpenText, Camera, Drone, EthernetPort, Globe, Menu, SquarePen } from 'lucide-react';
import type { LucideIcon } from 'lucide-react';
import type { TranslationKey } from '../i18n/index.ts';

type MenuItem =
  | { key: string; kind: 'route'; labelKey: TranslationKey; icon: LucideIcon; to: string }
  | { key: string; kind: 'external'; labelKey: TranslationKey; icon: LucideIcon; href: (lang: string) => string };

const { protocol, hostname } = window.location;

const MENU: MenuItem[] = [
  { key: 'dashboard', kind: 'route', labelKey: 'sidebar.dashboard', icon: Drone, to: '/' },
  { key: 'topics', kind: 'route', labelKey: 'sidebar.ros2', icon: EthernetPort, to: '/topics' },
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

export function Sidebar() {
  const [isOpen, setIsOpen] = useState(true);
  const { t, i18n } = useTranslation();

  const changeLanguage = (lang: string) => {
    void i18n.changeLanguage(lang);
    localStorage.setItem('language', lang);
  };

  return (
    <div
      className={`h-screen bg-gray-900 text-gray-100 flex flex-col transition-all duration-300 ${isOpen ? 'w-64' : 'w-20'}`}
    >
      {/* Header */}
      <div
        className={`flex items-center ${isOpen ? 'justify-between' : 'justify-center'} p-4 border-b border-gray-700`}
      >
        {isOpen && <span className="text-2xl font-bold">{t('header.title')}</span>}
        <button
          onClick={() => setIsOpen(!isOpen)}
          className="p-2 rounded-xl hover:bg-gray-800 transition"
        >
          <Menu size={20} />
        </button>
      </div>

      {/* Menu */}
      <nav className="flex-1 p-4 space-y-2">
        {MENU.map((item) => {
          const Icon = item.icon;
          const to = item.kind === 'route' ? item.to : item.href(i18n.language);

          return (
            <NavLink
              key={item.key}
              to={to}
              target={item.kind === 'external' ? '_blank' : undefined}
              className={({ isActive }) =>
                `flex items-center ${isOpen ? 'gap-3 justify-start' : 'justify-center'} p-2 rounded-xl transition ${
                  isActive ? 'bg-gray-700' : 'hover:bg-gray-800'
                }`
              }
            >
              <Icon size={20} />
              {isOpen && <span>{t(item.labelKey)}</span>}
            </NavLink>
          );
        })}
      </nav>

      {/* Language Selector */}
      <div
        className={`border-t border-gray-700 p-4 ${isOpen ? 'flex items-center justify-between' : 'flex justify-center'}`}
      >
        <Globe size={20} className="text-gray-400" />
        {isOpen && (
          <select
            value={i18n.language}
            onChange={(e) => changeLanguage(e.target.value)}
            className="bg-gray-800 text-gray-100 rounded-lg p-1 text-sm outline-none hover:bg-gray-700 transition"
          >
            {LANGUAGES.map((lang) => (
              <option key={lang.code} value={lang.code}>
                {lang.label}
              </option>
            ))}
          </select>
        )}
      </div>
    </div>
  );
}
