import type { Metadata } from "next";
import { Geist, Geist_Mono } from "next/font/google";
import Dock from "@/components/ui/navigation/Dock";
import "./globals.css";

const geistSans = Geist({
  variable: "--font-geist-sans",
  subsets: ["latin"],
});

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "Clover2",
  description: "Clover2 web interface",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="ru">
      <body
        className={`${geistSans.variable} ${geistMono.variable} antialiased`}
      >
        <main className="min-h-screen bg-gray-100">{children}</main>
        <Dock />
      </body>
    </html>
  );
}
