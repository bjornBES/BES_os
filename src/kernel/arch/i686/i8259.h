#pragma once

#include "pic.h"

const PICDriver* i8259_GetDriver();
void i8259_SendEOI(int irq);
