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
#include <sbi_utils/mailbox/rpmi_mailbox.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_heap.h>

struct mpxy_rpmi_mm {
	u32 mm_version;
	u32 shmem_addr_lo;
	u32 shmem_addr_hi;
	u32 shmem_size;
};

static int mpxy_rpmi_mm_xfer(void *context, struct mbox_chan *chan,
			     struct mbox_xfer *xfer)
{
	struct rpmi_message_args *args = xfer->args;
	struct mpxy_rpmi_mm *mmg = context;
	int rc = 0;

	if (!xfer->rx || (args->type != RPMI_MSG_NORMAL_REQUEST))
		return 0;

	switch (args->service_id) {
	case RPMI_MM_SRV_GET_ATTRIBUTES:
		((u32 *)xfer->rx)[0] = cpu_to_le32(RPMI_SUCCESS);
		((u32 *)xfer->rx)[1] = cpu_to_le32(mmg->mm_version);
		((u32 *)xfer->rx)[2] = cpu_to_le32(mmg->shmem_addr_lo);
		((u32 *)xfer->rx)[3] = cpu_to_le32(mmg->shmem_addr_hi);
		((u32 *)xfer->rx)[4] = cpu_to_le32(mmg->shmem_size);
		args->rx_data_len = 5 * sizeof(u32);
		break;

	case RPMI_MM_SRV_COMMUNICATE:
		sbi_printf("%s: case RPMI_MM_SRV_COMMUNICATE \n", __func__);
		rc = mbox_chan_xfer(chan, xfer);
		break;

	default:
		((u32 *)xfer->rx)[0] = cpu_to_le32(RPMI_ERR_NOTSUPP);
		args->rx_data_len = sizeof(u32);
		break;
	};

	return rc;
}

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

static int mpxy_rpmi_mm_setup(void **context, struct mbox_chan *chan,
			      const struct mpxy_rpmi_mbox_data *data)
{
	struct rpmi_mm_get_attributes_rsp resp;
	unsigned long mm_region_addr = 0;
	unsigned long mm_region_size = 0;
	struct mpxy_rpmi_mm *mmg;
	int rc = 0;

	rc = rpmi_normal_request_with_status(chan, RPMI_MM_SRV_GET_ATTRIBUTES,
					     NULL, 0, 0, &resp, rpmi_u32_count(resp),
					     rpmi_u32_count(resp));
	if (rc)
		return rc;

	mmg = sbi_zalloc(sizeof(*mmg));
	if (!mmg)
		return SBI_ENOMEM;

	mmg->mm_version = resp.mm_version;
	mmg->shmem_addr_lo = resp.shmem_addr_lo;
	mmg->shmem_addr_hi = resp.shmem_addr_hi;
	mmg->shmem_size = resp.shmem_size;

#if __riscv_xlen == 32
	mm_region_addr = resp.shmem_addr_lo;
#else
	mm_region_addr = (ulong)resp.shmem_addr_hi << 32 |  resp.shmem_addr_lo;
#endif

	mm_region_size = resp.shmem_size;

	rc = sbi_domain_root_add_memrange(mm_region_addr,
				mm_region_size,
				PAGE_SIZE, // Align at 1MB
				(SBI_DOMAIN_MEMREGION_MMIO |
				SBI_DOMAIN_MEMREGION_SHARED_SURW_MRW));
	if (rc)
		return rc;

	*context = mmg;

	return 0;
}

static const struct mpxy_rpmi_mbox_data mm_data = {
	.servicegrp_id = RPMI_SRVGRP_MM,
	.num_services = RPMI_MM_SRV_MAX_COUNT,
	.service_data = mm_services,
	.setup_group = mpxy_rpmi_mm_setup,
	.xfer_group = mpxy_rpmi_mm_xfer,
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
