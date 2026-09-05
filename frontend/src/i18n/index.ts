import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import en from './en.json';
import ru from './ru.json';

type DeepString<T> = { [K in keyof T]: T[K] extends string ? string : DeepString<T[K]> };

export type TranslationSchema = DeepString<typeof en>;

type DotPaths<T, Prefix extends string = ''> = {
  [K in keyof T]: T[K] extends string ? `${Prefix}${string & K}` : DotPaths<T[K], `${Prefix}${string & K}.`>;
}[keyof T];
export type TranslationKey = DotPaths<TranslationSchema>;

const ruMessages: TranslationSchema = ru;

declare module 'i18next' {
  interface CustomTypeOptions {
    defaultNS: 'translation';
    resources: { translation: TranslationSchema };
  }
}

i18n.use(initReactI18next).init({
  resources: {
    en: { translation: en },
    ru: { translation: ruMessages },
  },
  lng: localStorage.getItem('language') || 'ru',
  fallbackLng: 'ru',
  interpolation: { escapeValue: false },
});

i18n.on('languageChanged', (lng) => {
  document.documentElement.lang = lng;
});

export default i18n;
