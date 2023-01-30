#include "timing.h"

typedef struct time_context {
    double *time_segments;
    size_t num_segments;
} time_context_t;