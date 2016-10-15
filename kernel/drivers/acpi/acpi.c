#include <drivers/acpi.h>
#include <io.h>
#include <string.h>
#include <klog.h>
#include <drivers/pit.h>

uint32_t *smi_cmd;
uint8_t acpi_enable_g;
uint8_t acpi_disable_g;
uint32_t *pm1a_cnt;
uint32_t *pm1b_cnt;
uint16_t slp_typa;
uint16_t slp_typb;
uint16_t sci_en;
uint16_t slp_en;
uint8_t pm1_cnt_len;

uint32_t *acpi_check_rsd_ptr(uint32_t *ptr) {
	char *sig = "RSD PTR ";
	struct rsd_ptr *rsdp = (struct rsd_ptr*)ptr;
	uint8_t *bptr;
	uint8_t check = 0;
	uint32_t i = 0;

	if (memcmp(sig, rsdp, 8) == 0) {
		bptr = (uint8_t*)ptr;
		for (i = 0; i < sizeof(struct rsd_ptr); i++) {
			check += *bptr;
			bptr++;
		}

		if (check == 0)
			return (uint32_t*)rsdp->rsdt_addr;
	}

	return NULL;
}

uint32_t *acpi_get_rsd_ptr(void) {
	uint32_t *addr;
	uint32_t *rsdp;

	for (addr = (uint32_t *)0x000E0000; (int32_t)addr < 0x00100000; addr += 0x10 / sizeof(addr)) {
		rsdp = acpi_check_rsd_ptr(addr);
		if (rsdp != NULL)
			return rsdp;
	}

	int32_t ebda = *((int16_t*)0x40E);
	ebda = ebda * 0x10 & 0x000FFFFF;

	for (addr = (uint32_t *)ebda; (int32_t)addr < ebda + 1024; addr += 0x10 / sizeof(addr)) {
		rsdp = acpi_check_rsd_ptr(addr);
		if (rsdp != NULL)
			return rsdp;
	}

	return NULL;
}

bool acpi_check_header(uint32_t *ptr, char *sig) {

	if (memcmp(ptr, sig, 4) == 0) {
		int8_t *checkPtr = (int8_t *) ptr;
		int32_t len = *(ptr + 1);
		int8_t check = 0;
		while (0 < len--) {
			check += *checkPtr;
			checkPtr++;
		}

		if (check == 0)
			return 0;
	}

	return -1;
}

bool acpi_enable(void) {
	if ((io_inportw((uint32_t)pm1a_cnt) & sci_en) == 0) {
		if (smi_cmd != 0 && acpi_enable_g != 0) {
			io_outportb((uint32_t)smi_cmd, acpi_enable_g);
			uint32_t i = 0;
			for (i = 0; i < 300; i++) {
				if (io_inportw((uint32_t)pm1a_cnt) & sci_en)
				break;
				pit_wait_ms(10);
			}

			if (pm1b_cnt != 0)
				for (; i < 300; i++) {
					if ((io_inportw((uint32_t)pm1b_cnt) & sci_en) == 1)
					break;
					pit_wait_ms(10);
				}
			if (i < 300) {
				klog_detail("ACPI enabled.\n");
				return 0;
			} else {
				klog_warn("Failed to enable ACPI.\n");
				return 1;
			}
		} else {
			klog_warn("No known way to enable ACPI.\n");
			return 1;
		}
	} else
		klog_warn("ACPI was already enabled.\n");
	return 0;
}

bool acpi_init(void) {
	uint32_t *ptr = acpi_get_rsd_ptr();
	// check if address is correct  ( if acpi is available on this pc )
	if (ptr != NULL && acpi_check_header(ptr, "RSDT") == 0) {
		// the RSDT contains an unknown number of pointers to acpi tables
		int32_t entrys = *(ptr + 1);
		entrys = (entrys-36) /4;
		ptr += 36/4;   // skip header information

		while (0<entrys--) {
			// check if the desired table is reached
			if (acpi_check_header((uint32_t *) *ptr, "FACP") == 0) {
				entrys = -2;
				struct facp *facp = (struct facp *) *ptr;
				if (acpi_check_header((uint32_t *) facp->dsdt, "DSDT") == 0) {
					// search the \_S5 package in the DSDT
					char *S5Addr = (char *) facp->dsdt +36; // skip header
					int32_t dsdtLength = *(facp->dsdt+1) -36;
					while (0 < dsdtLength--) {
						if (memcmp(S5Addr, "_S5_", 4) == 0)
						break;
						S5Addr++;
					}
					// check if \_S5 was found
					if (dsdtLength > 0) {
						// check for valid AML structure
						if ( ( *(S5Addr-1) == 0x08 || ( *(S5Addr-2) == 0x08 && *(S5Addr-1) == '\\') ) && *(S5Addr+4) == 0x12 ) {
							S5Addr += 5;
							S5Addr += ((*S5Addr &0xC0)>>6) +2;   // calculate PkgLength size

							if (*S5Addr == 0x0A)
								S5Addr++;   // skip byteprefix
							slp_typa = *(S5Addr)<<10;
							S5Addr++;

							if (*S5Addr == 0x0A)
								S5Addr++;   // skip byteprefix
							slp_typb = *(S5Addr)<<10;

							smi_cmd = facp->smi_cmd;

							acpi_enable_g = facp->acpi_enable;
							acpi_disable_g = facp->acpi_disable;

							pm1a_cnt = facp->pm1a_cnt_blk;
							pm1b_cnt = facp->pm1b_cnt_blk;

							pm1_cnt_len = facp->pm1_cnt_len;

							slp_en = 1<<13;
							sci_en = 1;

							return 0;
						} else {
							klog_warn("ACPI: _S5 parse error.\n");
						}
					} else {
						klog_warn("ACPI: _S5 not present.\n");
					}
				} else {
					klog_warn("ACPI: DSDT invalid.\n");
				}
			}
			ptr++;
		}
		klog_warn("ACPI: No valid FACP present.\n");
	} else
		klog_warn("No ACPI available.\n");
	return -1;
}

void acpi_shutdown(void) {
	if (sci_en == 0)
		return;
	if (acpi_enable())
		return;

	io_outportw((uint32_t)pm1a_cnt, slp_typa | slp_en);
	if (pm1b_cnt != 0)
		io_outportw((uint32_t)pm1b_cnt, slp_typb | slp_en);

	klog_fatal("ACPI poweroff failed. System halting...\n");
}
