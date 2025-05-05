/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Sunil VL <sunilvl@ventanamicro.com>
 *   Ranbir Singh <rsingh@ventanamicro.com>
 */

#include <sbi_utils/mpxy/fdt_mpxy_rpmi_mbox.h>

struct mpxy_rpmi_mm {
	u32 mm_version;
	u32 shmem_addr_lo;
	u32 shmem_addr_hi;
	u32 shmem_size;
};

#if 0
static int mpxy_rpmi_mm_xfer(void *context, struct mbox_chan *chan,
			     struct mbox_xfer *xfer)
{
	struct rpmi_message_args *args = xfer->args;
	struct mpxy_rpmi_mm *smg = context;
	int rc = 0;

	if (!xfer->rx || (args->type != RPMI_MSG_NORMAL_REQUEST))
		return 0;

	switch (args->service_id) {
	case RPMI_MM_SRV_GET_ATTRIBUTES:
		((u32 *)xfer->rx)[0] = cpu_to_le32(RPMI_SUCCESS);
		((u32 *)xfer->rx)[1] = cpu_to_le32(smg->mm_version);
		((u32 *)xfer->rx)[2] = cpu_to_le32(smg->shmem_addr_lo);
		((u32 *)xfer->rx)[3] = cpu_to_le32(smg->shmem_addr_hi);
		((u32 *)xfer->rx)[4] = cpu_to_le32(smg->shmem_size);
		args->rx_data_len = 5 * sizeof(u32);
		break;

	default:
		((u32 *)xfer->rx)[0] = cpu_to_le32(RPMI_ERR_NOTSUPP);
		args->rx_data_len = sizeof(u32);
		break;
	};

	return rc;
}
#endif

static struct mpxy_rpmi_service_data mm_services[] = {
	[0] {
	     .id = RPMI_MM_SRV_GET_ATTRIBUTES,
	     .min_tx_len = 0,
	     .max_tx_len = 0,
	     .min_rx_len = sizeof(struct rpmi_mm_get_attributes_rsp),
	     .max_rx_len = sizeof(struct rpmi_mm_get_attributes_rsp),
	      },
	[1] {
	     .id = RPMI_MM_SRV_COMMUNICATE,
	     .min_tx_len = sizeof(struct rpmi_mm_communicate_req),
	     .max_tx_len = sizeof(struct rpmi_mm_communicate_req),
	     .min_rx_len = sizeof(struct rpmi_mm_communicate_rsp),
	     .max_rx_len = sizeof(struct rpmi_mm_communicate_rsp),
	      },
};

/*
 * Not using RPMI_MM_SRV_MAX_COUNT for .num_services field as Ventana is not
 * adding support for RPMI_MM_SRV_ENABLE_NOTIFICATION and hence the entry for
 * the same is not included in the mm_services[] look up table above.
 */
static const struct mpxy_rpmi_mbox_data mm_data = {
	.servicegrp_id = RPMI_SRVGRP_MM,
	.num_services = RPMI_MM_SRV_MAX_COUNT,
	.service_data = mm_services,
};

/* one extra blank entry for loop termination while matching */
static const struct fdt_match mm_match[] = {
	[0] {
	     .compatible = "riscv,rpmi-mpxy-mm",
	     .data = &mm_data,
	      },
	[1] {
	      },
};

const struct fdt_driver fdt_mpxy_rpmi_mm = {
	.match_table = mm_match,
	.init = mpxy_rpmi_mbox_init,
	.experimental = true,
};
