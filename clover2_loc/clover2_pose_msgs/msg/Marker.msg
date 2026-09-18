uint8 TYPE_FIXED=0
uint8 TYPE_STATIC=1
uint8 TYPE_DYNAMIC=2

# Unique marker id from dictionary
uint32 id
uint8 type

# Side size in meters
float32 size

string marker_frame_id

# Only if type is fixed
geometry_msgs/PoseWithCovariance pose
