"use client";

import { motion, useMotionValue, useSpring, useTransform, type MotionValue } from "motion/react";
import { useEffect, useMemo, useRef, useState, type MouseEvent as ReactMouseEvent } from "react";
import { useRouter } from "next/navigation";
import { useTranslation } from "react-i18next";
import { BookOpenText, Drone, EthernetPort, Languages, Settings, Terminal } from "lucide-react";

import "@/i18n";
import { getSocialLinks } from "@/components/ui/navigation/socialLinks";

type DockIconProps = {
  children: React.ReactNode;
  mouseX: MotionValue<number>;
  href?: string;
  target?: "_blank" | "_self";
  rel?: string;
  onClick?: () => void;
  label: string;
};

type NavItem = {
  label: string;
  icon: React.ReactNode;
  href?: string;
  onClick?: () => void;
  external?: boolean;
};

function DockIcon({ children, mouseX, href, target, rel, onClick, label }: DockIconProps) {
  const anchorRef = useRef<HTMLAnchorElement | null>(null);
  const buttonRef = useRef<HTMLButtonElement | null>(null);

  const distance = useTransform(mouseX, (val: number) => {
    const el = anchorRef.current ?? buttonRef.current;
    const bounds = el?.getBoundingClientRect() ?? {
      x: 0,
      width: 0,
    };
    return val - bounds.x - bounds.width / 2;
  });

  const widthSync = useTransform(distance, [-150, 0, 150], [40, 80, 40]);
  const width = useSpring(widthSync, {
    mass: 0.1,
    stiffness: 150,
    damping: 12,
  });

  if (href) {
    return (
      <motion.a
        ref={anchorRef}
        href={href}
        target={target}
        rel={rel}
        style={{ width }}
        className="aspect-square rounded-[12px] bg-neutral-800/80 backdrop-blur-md border border-neutral-700 flex items-center justify-center text-neutral-50 hover:bg-[#ff5e2b]/20 hover:border-[#ff5e2b]/40 hover:text-[#ff5e2b] transition-colors relative group cursor-pointer"
        whileHover={{ y: -8 }}
        whileTap={{ scale: 0.95 }}
        aria-label={label}
      >
        {children}
        <div className="absolute bottom-full mb-[8px] px-[12px] py-[6px] bg-neutral-900 border border-neutral-700 rounded-[6px] text-[12px] font-['Geoform:Regular',sans-serif] text-neutral-50 opacity-0 group-hover:opacity-100 transition-opacity pointer-events-none whitespace-nowrap">
          {label}
        </div>
      </motion.a>
    );
  }

  return (
    <motion.button
      ref={buttonRef}
      onClick={onClick}
      style={{ width }}
      className="aspect-square rounded-[12px] bg-neutral-800/80 backdrop-blur-md border border-neutral-700 flex items-center justify-center text-neutral-50 hover:bg-[#ff5e2b]/20 hover:border-[#ff5e2b]/40 hover:text-[#ff5e2b] transition-colors relative group cursor-pointer"
      whileHover={{ y: -8 }}
      whileTap={{ scale: 0.95 }}
      aria-label={label}
      type="button"
    >
      {children}
      <div className="absolute bottom-full mb-[8px] px-[12px] py-[6px] bg-neutral-900 border border-neutral-700 rounded-[6px] text-[12px] font-['Geoform:Regular',sans-serif] text-neutral-50 opacity-0 group-hover:opacity-100 transition-opacity pointer-events-none whitespace-nowrap">
        {label}
      </div>
    </motion.button>
  );
}

function DockDivider() {
  return (
    <div className="h-[48px] w-[2px] bg-linear-to-b from-transparent via-[#ff5e2b]/60 to-transparent mx-[6px]" />
  );
}

export default function Dock() {
  const mouseX = useMotionValue(Infinity);
  const router = useRouter();
  const { t, i18n } = useTranslation();
  const [language, setLanguage] = useState("ru");
  const [hostInfo, setHostInfo] = useState({ protocol: "", hostname: "" });

  const navigationItems: NavItem[] = useMemo(() => {
    const externalBase = hostInfo.hostname ? `${hostInfo.protocol}//${hostInfo.hostname}` : "";
    return [
      {
        icon: <Drone className="size-[20px]" />,
        label: t("sidebar.dashboard"),
        onClick: () => router.push("/"),
      },
      {
        icon: <EthernetPort className="size-[20px]" />,
        label: t("sidebar.ros2"),
        onClick: () => router.push("/ros2"),
      },
      {
        icon: <Settings className="size-[20px]" />,
        label: t("sidebar.settings"),
        onClick: () => router.push("/settings"),
      },
      {
        icon: <BookOpenText className="size-[20px]" />,
        label: t("sidebar.documentation"),
        href: externalBase ? `${externalBase}:81/${language}` : "#",
        external: true,
      },
      {
        icon: <Terminal className="size-[20px]" />,
        label: t("sidebar.terminal"),
        href: externalBase ? `${externalBase}:3000/wetty` : "#",
        external: true,
      },
      {
        icon: <Terminal className="size-[20px]" />,
        label: t("sidebar.camera"),
        href: externalBase ? `${externalBase}:8081` : "#",
        external: true,
      },
      {
        icon: <Languages className="size-[20px]" />,
        label: language === "ru" ? "Ru/En" : "En/Ru",
        onClick: () => {
          const next = language === "ru" ? "en" : "ru";
          setLanguage(next);
          window.localStorage.setItem("language", next);
          void i18n.changeLanguage(next);
        },
      },
    ];
  }, [hostInfo.hostname, hostInfo.protocol, i18n, language, router, t]);

  const socialItems = useMemo(() => getSocialLinks(), []);

  const handleMouseMove = (event: ReactMouseEvent<HTMLDivElement>) => {
    mouseX.set(event.pageX);
  };

  const handleMouseLeave = () => {
    mouseX.set(Infinity);
  };

  useEffect(() => {
    const stored = window.localStorage.getItem("language") || "ru";
    setLanguage(stored);
    void i18n.changeLanguage(stored);
    setHostInfo({
      protocol: window.location.protocol,
      hostname: window.location.hostname,
    });
  }, [i18n]);

  return (
    <div className="fixed bottom-[24px] left-1/2 -translate-x-1/2 z-50 hidden md:block">
      <motion.div
        onMouseMove={handleMouseMove}
        onMouseLeave={handleMouseLeave}
        className="flex items-center gap-[8px] px-[16px] py-[12px] bg-neutral-900/80 backdrop-blur-xl border border-neutral-700 rounded-[20px] shadow-2xl"
        initial={{ y: 100, opacity: 0 }}
        animate={{ y: 0, opacity: 1 }}
        transition={{ duration: 0.5, delay: 0.5 }}
      >
        {navigationItems.map((item, index) => (
          <DockIcon
            key={`${item.label}-${index}`}
            mouseX={mouseX}
            onClick={item.onClick}
            href={item.href}
            target={item.external ? "_blank" : undefined}
            rel={item.external ? "noopener noreferrer" : undefined}
            label={item.label}
          >
            {item.icon}
          </DockIcon>
        ))}

        <DockDivider />

        {socialItems.map((item) => (
          <DockIcon
            key={item.id}
            mouseX={mouseX}
            href={item.href}
            target="_blank"
            rel="noopener noreferrer"
            label={item.label}
          >
            {item.icon}
          </DockIcon>
        ))}
      </motion.div>
    </div>
  );
}
