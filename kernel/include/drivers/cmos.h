#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NMI_ENABLE 0
#define NMI_DISABLE 1
#define CMOS_WRITE 0x70
#define CMOS_READ 0x71
#define CMOS_REG(x) ((x) & (0x7F))
#define CMOS_RTC_BCD 0
#define CMOS_RTC_BINARY 4
#define CMOS_RTC_12H 0
#define CMOS_RTC_24H 2

#define CMOS_SECONDS CMOS_REG(0x00)
#define CMOS_MINUTES CMOS_REG(0x02)
#define CMOS_HOURS CMOS_REG(0x04)
#define CMOS_WEEKDAY CMOS_REG(0x06)
#define CMOS_DAY_OF_MONTH CMOS_REG(0x07)
#define CMOS_MONTH CMOS_REG(0x08)
#define CMOS_YEAR CMOS_REG(0x09)
#define CMOS_STATUS_A CMOS_REG(0x0A)
#define CMOS_STATUS_B CMOS_REG(0x0B)
#define CMOS_CENTURY

struct cmos_time_raw {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t days;
	uint8_t months;
	uint8_t years;
};

struct cmos_time {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t days;
	uint8_t months;
	uint32_t years;
};

void nmi_enable();
void nmi_disable();
struct cmos_time cmos_read();
