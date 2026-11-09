# Current activation lease holder, published by the clover2_fcu bridge
# (transient local, late-joining subscribers get the current state
# immediately).
#
# The lease is granted via ActivateMode and revoked reactively, only on
# FCU facts observed through the system_state mirror (mode != the leased
# one / armed = false / failsafe) or a broken mode bond.

# 0 = no active mode (lease surrendered)
uint32 mode_id

# Registered mode name, empty when no mode is active
string name
