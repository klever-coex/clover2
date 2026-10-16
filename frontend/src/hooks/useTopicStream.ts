import { useEffect } from 'react';
import { useRosStore } from '@/store/useRosStore';

export function useTopicStream(topicName: string | null) {
  const streamState = useRosStore((s) => s.streamState);
  const streamError = useRosStore((s) => s.streamError);
  const streamMessages = useRosStore((s) => s.streamMessages);
  const streamReceived = useRosStore((s) => s.streamReceived);
  const subscribeTopic = useRosStore((s) => s.subscribeTopic);
  const closeStream = useRosStore((s) => s.closeStream);
  const clearMessages = useRosStore((s) => s.clearMessages);
  const retryStream = useRosStore((s) => s.retryStream);

  useEffect(() => {
    if (topicName === null) return;
    subscribeTopic(topicName);
    return () => closeStream();
  }, [topicName, subscribeTopic, closeStream]);

  return {
    state: streamState,
    error: streamError,
    messages: streamMessages,
    received: streamReceived,
    clear: clearMessages,
    retry: retryStream,
  };
}
