/*  gnunetircd Distributed example.
 *  Copyright (C) 2017  Michael Mestnik <cheako+github_com@mikemestnik.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GNS_H_
#define GNS_H_

#include <gnunet/gnunet_config.h>
#include <gnunet/platform.h>
#include <gnunet/gnunet_util_lib.h>
#include "routing.h"

#define GNS_OP_NICK 1
#define GNS_OP_REAL 2
#define GNS_OP_PRID 3
#define GNS_OP_CHAN 4

GNUNET_NETWORK_STRUCT_BEGIN
/**
 * @ingroup gns
 * @brief header for DHT contents
 *
 * Used for accessing network globals
 */
struct IrcdGnsHeader {
	/**
	 * @ingroup gns
	 * @brief record type
	 *
	 * Used in case usage expands beyond nick and channel names, should be 0.
	 */
	unsigned char type;
	/**
	 * @ingroup gns
	 * @brief record version
	 *
	 * Should be 1.
	 */
	unsigned char version;
	/**
	 * @ingroup gns
	 * @brief number of opcode(s) following
	 */
	uint16_t o_count GNUNET_PACKED;
	/**
	 * @ingroup gns
	 * @brief opcodes and pointer to operands
	 *
	 * Instructions for IRCD to process regarding this record.
	 */
	struct IrcdGnsOpcode {
		/**
		 * @ingroup gns
		 * @brief opcode instruction
		 *
		 * Indicates instruction to process and length of operand(s).
		 */
		uint16_t code GNUNET_PACKED;
		/**
		 * @ingroup gns
		 * @brief offset of operand(s)
		 *
		 * First byte of first operand minus [IrcdDhtHeader::size].
		 */
		uint16_t offset GNUNET_PACKED;
	/**
	 * @ingroup gns
	 * @brief location used to access memory after structure
	 */
	} opcodes[];
};
GNUNET_NETWORK_STRUCT_END

void gns_lookup(const struct BaseRoutingNode *, struct BaseRoutingNode *,
		const char *);
void gns_publish(struct BaseRoutingNode *);
void gns_init(const struct GNUNET_CONFIGURATION_Handle *);

#endif /* GNS_H_ */
