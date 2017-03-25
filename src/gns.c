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

#include <stdint.h>
#include "gns.h"
#include <gnunet/gnunet_gnsrecord_lib.h>
#include <gnunet/gnunet_namestore_service.h>
#include <gnunet/gnunet_identity_service.h>
#include "hybrid-6/irc_string.h"

static struct GNUNET_NAMESTORE_Handle *ns;
static struct GNUNET_PeerIdentity pid;

static char *label_ucase(const char *str) {
	char *label, *t;
	label = GNUNET_malloc(NICKLEN + 13);
	strncpy(label, str, NICKLEN);
	t = label;
	while ((*t = ToUpper(*t)))
	t++;
	strncat(label, ".gnunet.gns.", NICKLEN + 12);
	return label;
}

/**
 * @brief publish gns record
 *
 * @param c nick, realname, and ecdsa key
 */
void gns_publish(struct BaseRoutingNode *brn) {
	GNUNET_NETWORK_STRUCT_BEGIN
	struct {
		struct GNUNET_GNSRECORD_Data rd;
		struct IrcdGnsHeader h;
		struct IrcdGnsOpcode o[3];
		struct GNUNET_PeerIdentity p;
		char d[NICKLEN + REALLEN + 2];
	} data;
	GNUNET_NETWORK_STRUCT_END

	size_t nicklen, reallen;
	nicklen = strlen(brn->nick);
	reallen = strlen(brn->real);

	struct GNUNET_TIME_Absolute expire;
	expire = GNUNET_TIME_absolute_add(GNUNET_TIME_absolute_get(),
			GNUNET_TIME_UNIT_DAYS);

	data.rd.data = &data.h;
	data.rd.data_size = sizeof(data.h) + sizeof(data.o);
	data.rd.data_size += sizeof(data.p) + nicklen + reallen + 2;
	data.rd.expiration_time = expire.abs_value_us;
	data.rd.record_type = 6667;
	data.rd.flags = GNUNET_GNSRECORD_RF_NONE;

	data.h.type = 0;
	data.h.version = 1;
	data.h.o_count = 3;

	data.o[0].code = htons(GNS_OP_NICK); /* 1 == Assign nick name from ascii string */
	data.o[0].offset = htons(sizeof(data.p));
	data.o[1].code = htons(GNS_OP_REAL); /* 2 == Assign real name from ascii string */
	data.o[1].offset = htons(sizeof(data.p) + strlen(brn->nick) + 1);
	data.o[2].code = htons(GNS_OP_PRID); /* 3 == Assign Peer Identity */
	data.o[2].offset = htons(0);

	data.p = pid;
	strcpy(data.d, brn->nick);
	strcpy(data.d + nicklen + 1, brn->real);

	char *label;
	label = label_ucase(brn->nick);

	const struct GNUNET_CRYPTO_EcdsaPrivateKey *key;
	key = GNUNET_IDENTITY_ego_get_private_key(brn->ego);

	GNUNET_NAMESTORE_records_store(ns, key, label, 1, &data.rd, NULL, NULL );
	GNUNET_free(label);
}

/**
 * @brief connect to namestore and gns services
 *
 * @param cfg used in connect calls and for peerid
 */
void gns_init(const struct GNUNET_CONFIGURATION_Handle *cfg) {
	GNUNET_assert(GNUNET_CRYPTO_get_peer_identity(cfg, &pid) == GNUNET_OK);
	ns = GNUNET_NAMESTORE_connect(cfg);
}
