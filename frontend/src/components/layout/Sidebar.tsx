import { Fragment, useState } from 'react';
import type { ReactNode } from 'react';
import { Link, NavLink, useLocation } from 'react-router';
import { useTranslation } from 'react-i18next';
import {
  BookOpenText,
  ChevronDown,
  ChevronRight,
  EthernetPort,
  Globe,
  Map,
  Network,
  ScrollText,
  Settings,
  SquarePen,
  Video,
} from 'lucide-react';
import type { LucideIcon } from 'lucide-react';
import type { TranslationKey } from '@/i18n/index.ts';
import { route, routeLabelKey } from '@/routes/navigation.ts';
import { usePageHeaderStore } from '@/store/usePageHeader';
import { cn } from '@/lib/utils';
import { docsBaseUrl } from '@/constants/docs';
import { Button } from '@/components/ui/button';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuGroup,
  DropdownMenuRadioGroup,
  DropdownMenuRadioItem,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import {
  Sidebar,
  SidebarContent,
  SidebarFooter,
  SidebarGroup,
  SidebarGroupLabel,
  SidebarHeader,
  SidebarInset,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  SidebarProvider,
  SidebarRail,
  SidebarTrigger,
  useSidebar,
} from '@/components/ui/sidebar';
import { Separator } from '@/components/ui/separator';

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

const ideUrl = () => {
  const { protocol, hostname } = window.location;
  return `${protocol}//${hostname}:9880/?folder=/home/pi`;
};

const MENU: NavSection[] = [
  {
    key: 'main',
    items: [
      { key: 'map', labelKey: route('/map').labelKey, icon: Map, to: '/map' },
      { key: 'video', labelKey: route('/video').labelKey, icon: Video, to: '/video' },
      { key: 'settings', labelKey: route('/settings').labelKey, icon: Settings, to: '/settings' },
    ],
  },
  {
    key: 'ros2',
    labelKey: 'sidebar.ros2',
    items: [
      { key: 'nodes', labelKey: route('/ros2/nodes').labelKey, icon: Network, to: '/ros2/nodes' },
      { key: 'topics', labelKey: route('/ros2/topics').labelKey, icon: EthernetPort, to: '/ros2/topics' },
      { key: 'logs', labelKey: route('/ros2/logs').labelKey, icon: ScrollText, to: '/ros2/logs' },
    ],
  },
  {
    key: 'external',
    items: [
      {
        key: 'documentation',
        labelKey: 'sidebar.documentation',
        icon: BookOpenText,
        to: (lang) => docsBaseUrl(lang) + 'index.html',
      },
      {
        key: 'ide',
        labelKey: 'sidebar.ide',
        icon: SquarePen,
        to: ideUrl,
      },
    ],
  },
];

const LANGUAGES = [
  { code: 'en', label: 'English' },
  { code: 'ru', label: 'Русский' },
] as const;

const COLLAPSED_STORAGE_KEY = 'sidebar-collapsed';

function AppSidebar() {
  const { t, i18n } = useTranslation();
  const { pathname } = useLocation();
  const { state } = useSidebar();
  const isOpen = state === 'expanded';

  const changeLanguage = (lang: string) => {
    void i18n.changeLanguage(lang);
    localStorage.setItem('language', lang);
  };

  const isItemActive = (item: NavLinkItem) =>
    typeof item.to === 'string' &&
    (pathname === item.to || pathname.startsWith(`${item.to}/`));

  const renderLink = (item: NavLinkItem) => {
    const Icon = item.icon;
    const label = t(item.labelKey);

    if (typeof item.to === 'string') {
      return (
        // NavLink sets aria-current="page" on the active route itself.
        <SidebarMenuItem key={item.key}>
          <SidebarMenuButton
            asChild
            isActive={isItemActive(item)}
            tooltip={label}
            className="rounded-row hover:bg-muted/60 hover:text-foreground data-active:bg-secondary data-active:text-foreground"
          >
            <NavLink to={item.to}>
              <Icon />
              <span>{label}</span>
            </NavLink>
          </SidebarMenuButton>
        </SidebarMenuItem>
      );
    }

    return (
      <SidebarMenuItem key={item.key}>
        <SidebarMenuButton
          asChild
          tooltip={label}
          className="rounded-row hover:bg-muted/60 hover:text-foreground"
        >
          <a href={item.to(i18n.language)} target="_blank" rel="noreferrer">
            <Icon />
            <span>{label}</span>
          </a>
        </SidebarMenuButton>
      </SidebarMenuItem>
    );
  };

  return (
    <Sidebar collapsible="icon">
      <SidebarHeader>
        <div className="flex h-12 items-center gap-2 px-2">
          <img
            src="/coex.svg"
            alt="COEX"
            className="size-8 max-w-none shrink-0 transition-[width,height] duration-fast ease-linear group-data-[collapsible=icon]:size-4"
          />
          <span className="pl-1 text-2xl font-bold group-data-[collapsible=icon]:hidden">
            {t('header.title')}
          </span>
        </div>
      </SidebarHeader>

      <SidebarContent>
        {MENU.map((section) => (
          <SidebarGroup key={section.key}>
            {section.labelKey !== undefined && (
              <SidebarGroupLabel>{t(section.labelKey)}</SidebarGroupLabel>
            )}
            <SidebarMenu className="gap-1">{section.items.map(renderLink)}</SidebarMenu>
          </SidebarGroup>
        ))}
      </SidebarContent>

      <SidebarFooter className="border-t border-border pb-3">
        <DropdownMenu>
          <DropdownMenuTrigger asChild>
            <Button
              variant="ghost"
              size={isOpen ? 'default' : 'icon'}
              aria-label={t('sidebar.language')}
              className={cn(
                'group/lang w-full text-muted-foreground hover:text-foreground',
                isOpen ? 'justify-start gap-2' : 'mx-auto',
              )}
            >
              <Globe data-icon="inline-start" />
              {isOpen && (
                <span className="text-sm font-normal">
                  {LANGUAGES.find((lang) => lang.code === i18n.language)?.label ??
                    i18n.language}
                </span>
              )}
              {isOpen && (
                <ChevronDown
                  data-icon="inline-end"
                  className="ml-auto transition-transform duration-fast group-data-[state=open]/lang:rotate-180"
                />
              )}
            </Button>
          </DropdownMenuTrigger>
          <DropdownMenuContent side="top" align="start" className="min-w-36">
            <DropdownMenuGroup>
              <DropdownMenuRadioGroup
                value={i18n.language}
                onValueChange={(lang) => changeLanguage(lang)}
              >
                {LANGUAGES.map((lang) => (
                  <DropdownMenuRadioItem key={lang.code} value={lang.code}>
                    {lang.label}
                  </DropdownMenuRadioItem>
                ))}
              </DropdownMenuRadioGroup>
            </DropdownMenuGroup>
          </DropdownMenuContent>
        </DropdownMenu>
      </SidebarFooter>
      <SidebarRail />
    </Sidebar>
  );
}

export function SidebarShell({ children }: { children: ReactNode }) {
  const { t } = useTranslation();
  const { pathname } = useLocation();
  const crumbs = usePageHeaderStore((s) => s.crumbs);
  const pageActions = usePageHeaderStore((s) => s.actions);
  const [open, setOpen] = useState(
    () => localStorage.getItem(COLLAPSED_STORAGE_KEY) !== 'true',
  );

  const fallbackKey = routeLabelKey(pathname);
  const items =
    crumbs.length > 0
      ? crumbs
      : fallbackKey !== undefined
        ? [{ label: t(fallbackKey) }]
        : [];

  return (
    <SidebarProvider
      open={open}
      onOpenChange={(next) => {
        setOpen(next);
        localStorage.setItem(COLLAPSED_STORAGE_KEY, String(!next));
      }}
    >
      <AppSidebar />
      <SidebarInset id="main-content" className="h-svh overflow-hidden">
        <header className="flex h-16 shrink-0 items-center gap-2 border-b border-border px-4">
          <SidebarTrigger aria-label={t('sidebar.toggleMenu')} className="-ml-1" />
          <Separator orientation="vertical" className="mr-1 !h-4" />
          <nav aria-label="Breadcrumb" className="flex min-w-0 items-center gap-2">
            {items.map((item, index) => {
              const last = index === items.length - 1;
              const label = (
                <span
                  className={cn(
                    'truncate text-sm',
                    item.mono === true && 'font-mono',
                    last ? 'font-medium text-foreground' : 'text-muted-foreground',
                  )}
                >
                  {item.label}
                </span>
              );
              return (
                <Fragment key={`${index}-${item.label}`}>
                  {index > 0 && (
                    <ChevronRight className="size-3.5 shrink-0 text-muted-foreground/60" />
                  )}
                  {!last && item.to !== undefined ? (
                    <Link
                      to={item.to}
                      className="truncate text-sm transition-colors duration-fast hover:text-foreground"
                    >
                      {label}
                    </Link>
                  ) : (
                    label
                  )}
                </Fragment>
              );
            })}
          </nav>
          {/* Pages have no visible heading — the breadcrumb tail is the title. */}
          <h1 className="sr-only">
            {items[items.length - 1]?.label ?? t('header.title')}
          </h1>
          {pageActions !== null && (
            <div className="ml-auto flex shrink-0 items-center gap-2">{pageActions}</div>
          )}
        </header>
        <div className="min-h-0 flex-1 overflow-y-auto">{children}</div>
      </SidebarInset>
    </SidebarProvider>
  );
}
