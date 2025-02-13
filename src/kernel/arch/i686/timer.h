#pragma once

#include <stdint.h>

#include "pit.h"
#include "irq.h"

void timer_phase(int hz);
void timer_handler(Registers *r);
void timer_wait(int ticks);
void sleepMs(int ms);
void sleepSec(int sec);