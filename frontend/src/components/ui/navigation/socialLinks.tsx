"use client";

import type { ReactNode } from "react";
import { GitBranch, Gitlab, Send, Youtube } from "lucide-react";

export type SocialLink = {
  id: "gitlab" | "gitverse" | "youtube" | "telegram";
  label: string;
  href: string;
  icon: ReactNode;
};

export function getSocialLinks(): SocialLink[] {
  return [
    {
      id: "gitlab",
      label: "GitLab",
      href: process.env.NEXT_PUBLIC_GITLAB_URL || "#",
      icon: <Gitlab className="size-[20px]" />,
    },
    {
      id: "gitverse",
      label: "Gitverse",
      href: process.env.NEXT_PUBLIC_GITVERSE_URL || "#",
      icon: <GitBranch className="size-[20px]" />,
    },
    {
      id: "youtube",
      label: "YouTube",
      href: process.env.NEXT_PUBLIC_YOUTUBE_URL || "#",
      icon: <Youtube className="size-[20px]" />,
    },
    {
      id: "telegram",
      label: "Telegram",
      href: process.env.NEXT_PUBLIC_TELEGRAM_URL || "#",
      icon: <Send className="size-[20px]" />,
    },
  ];
}
