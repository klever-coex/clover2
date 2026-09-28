import { VIDEO_SERVER_PORT } from '../constants/ros.ts';

export const VIDEO_IMAGE_TYPES = [
  'sensor_msgs/msg/Image',
  'sensor_msgs/msg/CompressedImage',
];

export function isVideoTopic(type: string): boolean {
  return VIDEO_IMAGE_TYPES.includes(type);
}

export function videoStreamUrl(topicName: string, codec: 'h264' | 'vp8' | 'vp9' = 'vp8'): string {
  const { protocol, hostname } = window.location;
  return `${protocol}//${hostname}:${VIDEO_SERVER_PORT}/stream?topic=${topicName}&type=${codec}`;
}
