#include <stdint.h>

#define SERVO_OPEN   (1 << 0)
#define SERVO_CLOSE  (1 << 1)
#define SERVO_MANUAL (1 << 2)

typedef struct {
    uint8_t status;
    uint8_t reserved;
    uint16_t value;
} FrameServo;

typedef union {
    FrameServo frame;
    uint8_t byteMap[sizeof(FrameServo)];
} ServoPacket;