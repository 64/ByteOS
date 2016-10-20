#include <io.h>
#include <string.h>
#include <klog.h>
#include <drivers/cmos.h>

#define CURRENT_CENTURY 21
#define BCD_TO_BINARY(val) (((val / 16) * 10) + (val & 0x0F))

static const uint8_t cmos_time_ports[] = {
	CMOS_SECONDS,
	CMOS_MINUTES,
	CMOS_HOURS,
	CMOS_DAY_OF_MONTH,
	CMOS_MONTH,
	CMOS_YEAR
};

bool cmos_nmi_value = NMI_ENABLE;

void nmi_enable() {
	cmos_nmi_value = NMI_ENABLE;
}

void nmi_disable() {
	cmos_nmi_value = NMI_DISABLE;
}

static inline uint8_t cmos_get_reg(uint8_t reg_num) {
	io_outportb(CMOS_WRITE, cmos_nmi_value | reg_num);
	return io_inportb(CMOS_READ);
}

static inline bool cmos_update_in_progress() {
	io_outportb(CMOS_WRITE, cmos_nmi_value | CMOS_STATUS_A);
	return (io_inportb(CMOS_READ) & 0x80);
}

void cmos_dump(uint8_t *addr) {
	uint32_t i;
	for (i = 0; i < (sizeof(cmos_time_ports) / sizeof(uint8_t)); i++) {
		*addr++ = cmos_get_reg(cmos_time_ports[i]);
	}
}

struct cmos_time cmos_read() {
	while (cmos_update_in_progress())
		;

	struct cmos_time_raw old_time, new_time;
	struct cmos_time rv;
	memset(&old_time, 0, sizeof(struct cmos_time_raw));
	memset(&new_time, 0, sizeof(struct cmos_time_raw));
	memset(&new_time, 0, sizeof(struct cmos_time));
	cmos_dump((uint8_t*)&old_time);

	do {
		memcpy(&old_time, &new_time, sizeof(struct cmos_time_raw));
		while (cmos_update_in_progress())
			;
		cmos_dump((uint8_t*)&new_time);
	} while((old_time.seconds != new_time.seconds) ||
		(old_time.minutes != new_time.minutes) ||
		(old_time.hours != new_time.hours) ||
                (old_time.days != new_time.days) ||
		(old_time.months != new_time.months) ||
		(old_time.years != new_time.years)
	);

	uint8_t b_reg = cmos_get_reg(CMOS_STATUS_B);

	if ((b_reg & 0x04) == CMOS_RTC_BINARY) {
		new_time.seconds = BCD_TO_BINARY(new_time.seconds);
		new_time.minutes = BCD_TO_BINARY(new_time.minutes);
		new_time.hours = ((new_time.hours & 0x0F) +
				 (((new_time.hours & 0x70) / 16) * 10) ) |
				 (new_time.hours & 0x80);
		new_time.days = BCD_TO_BINARY(new_time.days);
		new_time.months = BCD_TO_BINARY(new_time.months);
		new_time.years = BCD_TO_BINARY(new_time.years);
	}

	if ((b_reg & 0x02) == CMOS_RTC_24H) {
		new_time.hours = ((new_time.hours & 0x7F) + 12) % 24;
	}

	rv.seconds = new_time.seconds;
	rv.minutes = new_time.minutes;
	rv.hours = new_time.hours;
	rv.days = new_time.days;
	rv.months = new_time.months;
	rv.years = new_time.years + 2000;
	return rv;

}
