uint8 TYPE_ARUCO=0

# Unique marker id from dictionary
uint32 id
uint8 type

# Side size in meters
float32 size

string marker_frame_id

# Marker position
geometry_msgs/PoseWithCovariance pose
