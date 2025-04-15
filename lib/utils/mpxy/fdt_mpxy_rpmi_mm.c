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
	.num_services = array_size(mm_services),
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
