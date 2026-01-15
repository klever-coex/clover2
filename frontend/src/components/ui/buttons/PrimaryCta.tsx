"use client";

import clsx from "clsx";
import { useRouter } from "next/navigation";
import type { ReactNode } from "react";

type PrimaryCtaProps = {
  label: string;
  href?: string;
  to?: string;
  className?: string;
  startIcon?: ReactNode;
  endIcon?: ReactNode;
  action?: () => void;
  width?: "auto" | "full";
  textTone?: "dark" | "light";
  download?: boolean;
};

const baseClasses =
  "inline-flex items-center gap-[8px] px-[20px] py-[12px] rounded-[4px] bg-[#ff5e2b] border border-transparent hover:border-[#ff5e2b] hover:bg-[#ff5e2b]/90 transition-colors";

export default function PrimaryCta({
  label,
  href,
  to,
  className,
  startIcon,
  endIcon,
  action,
  width = "auto",
  textTone = "dark",
  download = false,
}: PrimaryCtaProps) {
  const router = useRouter();
  const fontClasses = "ty-button cursor-pointer";
  const widthClasses = width === "full" ? "w-full justify-center" : "";
  const textClasses = textTone === "light" ? "text-white" : "text-[#0f1011]";

  if (href) {
    return (
      <a
        href={href}
        target={download ? undefined : "_blank"}
        rel={download ? undefined : "noopener noreferrer"}
        download={download || undefined}
        className={clsx(baseClasses, fontClasses, widthClasses, textClasses, className)}
      >
        {startIcon}
        <span>{label}</span>
        {endIcon}
      </a>
    );
  }

  if (to) {
    const normalized = to.startsWith("/") ? to : `/${to}`;
    return (
      <button
        type="button"
        onClick={() => router.push(normalized)}
        className={clsx(baseClasses, fontClasses, widthClasses, textClasses, className)}
      >
        {startIcon}
        <span>{label}</span>
        {endIcon}
      </button>
    );
  }

  return (
    <button
      type="button"
      onClick={action}
      className={clsx(baseClasses, fontClasses, widthClasses, textClasses, className)}
    >
      {startIcon}
      <span>{label}</span>
      {endIcon}
    </button>
  );
}
