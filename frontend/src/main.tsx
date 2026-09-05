import '@fontsource/inter/400.css';
import '@fontsource/inter/500.css';
import '@fontsource/inter/600.css';
import '@fontsource/inter/700.css';
import '@fontsource/jetbrains-mono/400.css';
import '@fontsource/jetbrains-mono/500.css';
import '@fontsource/jetbrains-mono/600.css';
import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { BrowserRouter } from 'react-router';
import './index.css';
import './i18n';
import App from './App.tsx';

// TEMP DEBUG: surface runtime crashes visually (remove after diagnosis)
window.addEventListener('error', (e) => {
  const el = document.createElement('pre');
  el.style.cssText = 'position:fixed;top:0;left:0;z-index:99999;background:#fff;color:#900;padding:12px;white-space:pre-wrap;max-width:100vw';
  el.textContent = `ERR: ${e.message}\n${e.error?.stack ?? ''}`;
  document.body.appendChild(el);
});

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <BrowserRouter>
      <App />
    </BrowserRouter>
  </StrictMode>,
);
