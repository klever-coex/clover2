import { Component } from 'react';
import type { ReactNode } from 'react';

import i18n from '@/i18n/index.ts';
import { ErrorState } from '../common/ErrorState.tsx';

interface Props {
  children: ReactNode;
  fallback?: ReactNode;
}

interface State {
  hasError: boolean;
  error: Error | null;
}

export class ErrorBoundary extends Component<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = { hasError: false, error: null };
  }

  static getDerivedStateFromError(error: Error): State {
    return { hasError: true, error };
  }

  render() {
    if (this.state.hasError) {
      return (
        this.props.fallback ?? (
          <div className="h-full flex items-center justify-center p-6">
            <ErrorState
              message={this.state.error?.message ?? i18n.t('common.unexpectedError')}
              onRetry={() => this.setState({ hasError: false, error: null })}
            />
          </div>
        )
      );
    }

    return this.props.children;
  }
}
