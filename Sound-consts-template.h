// SoundConsts.h

#define SOUND_PORT  portb
#define SOUND_TRIS  trisb
#define SOUND_PIN  4

#define SOUND_MASK  (1 << SOUND_PIN)

// Define this on 18F architecture.
//#define SOUND_LATCH  latb
// Define this instead on 16F architecture.
#define SOUND_SHADOW
