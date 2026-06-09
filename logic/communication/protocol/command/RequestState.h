#include <stdint.h>

#define RS_FLAG_GET_SYSTEM_AFTER (1 << 0)
#define RS_FLAG_RESET            (1 << 1)
#define RS_FLAG_START_LOGGING    (1 << 2)
#define RS_FLAG_SPREAD           (1 << 3)

typedef struct {
    uint8_t flags;
    uint8_t requestedID;
} RequestStateFrame;

typedef union {
    RequestStateFrame frame;
    uint8_t byteMap[sizeof(RequestStateFrame)];
}RequestState;