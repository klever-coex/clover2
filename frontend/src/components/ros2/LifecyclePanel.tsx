import { useState } from 'react';
import { useTranslation } from 'react-i18next';

import { clover2Api } from '@/api/clover2.ts';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Item, ItemActions, ItemContent, ItemTitle } from '@/components/ui/item';
import { Spinner } from '@/components/ui/spinner';
import { InfoLink } from '../common/InfoLink.tsx';
import { useApiErrorMessage } from '@/hooks/useApiErrorMessage';
import { useAsyncResource } from '@/hooks/useAsyncResource';
import { toApiError } from '@/types/errors';
import type { LifecycleTransitionDescription } from '@/types/lifecycle';
import type { NodeInfo } from '@/types/node';
import { confirmDialog } from '@/store/useConfirmStore';
import { ResourceState } from '../common/ResourceState.tsx';
import { LifecycleBadge } from './LifecycleBadge.tsx';

/** Transitions that stop or discard the node — applied only after confirmation. */
const DESTRUCTIVE_TRANSITIONS = /(^|_)(shutdown|destroy)$/;

interface LifecyclePanelProps {
  nodeName: string;
  info: NodeInfo;
  onTransitioned: () => void;
}

export function LifecyclePanel({ nodeName, info, onTransitioned }: LifecyclePanelProps) {
  const { t } = useTranslation();
  const errorMessage = useApiErrorMessage();

  const transitions = useAsyncResource(
    () => clover2Api.nodes.availableTransitions(nodeName),
    [nodeName],
  );

  const [pending, setPending] = useState<string | null>(null);
  const [actionError, setActionError] = useState<string | null>(null);

  async function apply(transition: string) {
    if (DESTRUCTIVE_TRANSITIONS.test(transition)) {
      const confirmed = await confirmDialog({
        title: t('nodes.transitionConfirmTitle'),
        message: t('nodes.transitionConfirmMessage', {
          label: transition,
          node: nodeName,
        }),
        tone: 'danger',
        confirmLabel: t('nodes.applyTransitionConfirm'),
      });
      if (!confirmed) return;
    }

    setPending(transition);
    setActionError(null);

    try {
      await clover2Api.nodes.transition(nodeName, transition);
      transitions.reload();
      onTransitioned();
    } catch (error) {
      setActionError(errorMessage(toApiError(error)));
    } finally {
      setPending(null);
    }
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle className="flex items-center gap-2">
          {t('nodes.lifecycleControl')}
          <InfoLink docKey="ros2.node.lifecycle" />
        </CardTitle>
      </CardHeader>
      <CardContent>
        <div className="flex flex-col gap-3">
          <div className="flex items-center justify-between gap-4">
            <span className="text-sm text-muted-foreground">
              {t('nodes.currentState')}
            </span>
            <LifecycleBadge state={info.lifecycle_state?.label} />
          </div>

          {actionError !== null && (
            <p className="text-sm text-destructive" role="alert">
              {actionError}
            </p>
          )}

          <ResourceState
            resource={transitions}
            emptyMessage={t('nodes.noTransitions')}
          >
            {(list) => (
              <div className="flex flex-col gap-1">
                {list.map((description) => (
                  <TransitionButton
                    key={description.transition.label}
                    description={description}
                    busy={pending !== null}
                    active={pending === description.transition.label}
                    onApply={() => apply(description.transition.label)}
                  />
                ))}
              </div>
            )}
          </ResourceState>
        </div>
      </CardContent>
    </Card>
  );
}

interface TransitionButtonProps {
  description: LifecycleTransitionDescription;
  busy: boolean;
  active: boolean;
  onApply: () => void;
}

function TransitionButton({ description, busy, active, onApply }: TransitionButtonProps) {
  return (
    <Item size="sm" variant="outline" asChild>
      <button
        type="button"
        disabled={busy}
        onClick={onApply}
        className="cursor-pointer hover:bg-muted disabled:pointer-events-none disabled:opacity-50"
      >
        <ItemContent>
          <ItemTitle className="min-w-0 break-all font-mono leading-5">
            {description.transition.label}
          </ItemTitle>
        </ItemContent>
        <ItemActions>
          {active && <Spinner className="text-muted-foreground" />}
          <LifecycleBadge state={description.goal_state.label} />
        </ItemActions>
      </button>
    </Item>
  );
}
