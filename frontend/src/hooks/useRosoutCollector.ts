import { useRosStore } from '@/store/useRosStore';
import { useCapabilityFetch } from './useCapabilityFetch.ts';
import { useRosCapability } from './useRosCapability.ts';

export function useRosoutCollector(): void {
  const capability = useRosCapability('topics');
  const startRosoutStream = useRosStore((s) => s.startRosoutStream);
  useCapabilityFetch(capability, startRosoutStream);
}
