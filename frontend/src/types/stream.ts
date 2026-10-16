/** One ROS message deserialized to JSON. rclcpp serialization is acyclic, so no recursion guard is needed. */
export type RosJsonValue =
  | null
  | boolean
  | number
  | string
  | RosJsonValue[]
  | { [field: string]: RosJsonValue };
