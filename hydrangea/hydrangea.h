#ifndef HYDRANGEA_H
#define HYDRANGEA_H

#include <stdint.h>

#define TASK_DEF(func_ptr, interval, offset) {func_ptr, -(interval-offset), interval}

struct htask {
	const void (*run)(void);
	volatile uint16_t last_ms;
	const uint16_t interval_ms;
};

void hydrangea_init(void);

void hydrangea_run(struct htask arr[], uint8_t count);

uint16_t hydrangea_tick(void);

#endif /* HYDRANGEA_H */
