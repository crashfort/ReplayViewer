#if defined REC_INCLUDED
#endinput
#endif
#define REC_INCLUDED

// SVR functions.

#define REC_STATE_IDLE 0
#define REC_STATE_LOADED 1
#define REC_STATE_PLAYING 2
#define REC_STATE_FINISHED 3
#define REC_STATE_ERROR 999

// Mark a state in the replay process to notify SVR.
native void Rec_MarkState(int state);

// Give replay data back to SVR.
native void Rec_ProvideData(float velo[3], int buttons);

// Create a dummy navigation file so it doesn't take ages to load new maps.
native void Rec_CreateDummyNavForMap(char[] dummy_nav_path);
