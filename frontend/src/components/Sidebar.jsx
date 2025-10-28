import { useState } from "react";
import { NavLink } from "react-router-dom";
import { useTranslation } from "react-i18next";
import {
  Drone,
  Settings,
  EthernetPort,
  Menu,
  Globe,
  BookOpenText,
  Terminal
} from "lucide-react";

const Sidebar = () => {
  const [isOpen, setIsOpen] = useState(true);
  const [language, setLanguage] = useState(localStorage.getItem("language") || "en");
  const { t, i18n } = useTranslation();

  const changeLanguage = (lang) => {
    i18n.changeLanguage(lang);
    setLanguage(lang);
    localStorage.setItem("language", lang);
  };

  const menu = [
    { name: t("sidebar.dashboard"), icon: <Drone size={20} />, path: "/" },
    { name: t("sidebar.ros2"), icon: <EthernetPort size={20} />, path: "/ros2" },
    { name: t("sidebar.settings"), icon: <Settings size={20} />, path: "/settings" },
    {
      name: t("sidebar.documentation"),
      icon: <BookOpenText size={20} />,
      path: `${window.location.protocol}//${window.location.hostname}:81/${language}`,
      redirect: true
    },
    {
      name: t("sidebar.terminal"),
      icon: <Terminal size={20} />,
      path: `${window.location.protocol}//${window.location.hostname}:3000/wetty`,
      redirect: true
    },
  ];

  const languages = [
    { code: "en", label: "English" },
    { code: "ru", label: "Русский" },
  ];

  return (
    <div className={`h-screen bg-gray-900 text-gray-100 flex flex-col transition-all duration-300 ${isOpen ? "w-64" : "w-20"}`}>
      {/* Header */}
      <div className={`flex items-center ${isOpen ? "justify-between" : "justify-center"} p-4 border-b border-gray-700`}>
        {isOpen && <span className="text-2xl font-bold">{t("header.title")}</span>}
        <button onClick={() => setIsOpen(!isOpen)} className="p-2 rounded-xl hover:bg-gray-800 transition">
          <Menu size={20} />
        </button>
      </div>

      {/* Menu */}
      <nav className="flex-1 p-4 space-y-2">
        {menu.map((item) => (
          <NavLink
            target={item.redirect ? "_blank" : ""}
            key={item.name}
            to={item.path}
            className={({ isActive }) =>
              `flex items-center ${isOpen ? "gap-3 justify-start" : "justify-center"} p-2 rounded-xl transition ${isActive ? "bg-gray-700" : "hover:bg-gray-800"
              }`
            }
          >
            {item.icon}
            {isOpen && <span>{item.name}</span>}
          </NavLink>
        ))}
      </nav>

      {/* Language Selector */}
      <div className={`border-t border-gray-700 p-4 ${isOpen ? "flex items-center justify-between" : "flex justify-center"}`}>
        <Globe size={20} className="text-gray-400" />
        {isOpen && (
          <select
            value={language}
            onChange={(e) => changeLanguage(e.target.value)}
            className="bg-gray-800 text-gray-100 rounded-lg p-1 text-sm outline-none hover:bg-gray-700 transition"
          >
            {languages.map((lang) => (
              <option key={lang.code} value={lang.code}>
                {lang.label}
              </option>
            ))}
          </select>
        )}
      </div>
    </div>
  );
};

export default Sidebar;
