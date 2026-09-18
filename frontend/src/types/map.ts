// Wire types of the clover2_http map_server plugin (data/*.hpp).

export interface MarkerPose {
  x: number;
  y: number;
  z: number;
  roll: number;
  pitch: number;
  yaw: number;
}

export interface MarkerInfo {
  id: number;
  type: string;
  size: number;
  marker_frame_id: string;
  /** Present only for fixed markers. */
  pose?: MarkerPose;
}

export interface MapInfo {
  valid: boolean;
  name: string;
  frame_id: string;
  dictionary: string;
  count: number;
  markers: MarkerInfo[];
}

export interface ModifyResult {
  success: boolean;
  error_message: string;
}
