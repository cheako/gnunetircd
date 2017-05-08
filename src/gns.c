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
#include "mesh.h"
#include "dht.h"
#include <gnunet/gnunet_gnsrecord_lib.h>
#include <gnunet/gnunet_namestore_service.h>
#include <gnunet/gnunet_gns_service.h>
#include <gnunet/gnunet_identity_service.h>
#include "hybrid-6/irc_string.h"

struct GNUNET_GNS_Handle *h;
static struct GNUNET_NAMESTORE_Handle *ns;
static struct GNUNET_PeerIdentity pid;

void LookupResultProcessor(void *cls, uint32_t rd_count,
		const struct GNUNET_GNSRECORD_Data *rd) {
	struct BaseRoutingNode *brn = cls;
	NEXT_RECORD: while (rd_count--) {
		if (rd->record_type != 6667) {
			rd++;
			continue;
		}

		const struct IrcdGnsHeader *gns = rd->data;
		if (gns->type == 0 && gns->version <= 1) {
			uint16_t nopcodes = ntohs(gns->o_count);
			size_t hsize = sizeof(struct IrcdGnsHeader)
					+ sizeof(struct IrcdGnsOpcode) * nopcodes;
			for (int i = 0; i < nopcodes; i++) {
				int offset = hsize + ntohs(gns->opcodes[i].offset);
				switch (ntohs(gns->opcodes[i].code)) {
				case GNS_OP_CHAN:
				case GNS_OP_NICK: {
					char *name = (void *) gns + offset;
					if (routing_query(name) != brn) {
						rd++;
						goto NEXT_RECORD;
					}
					strncpy(brn->name, name, CHANNELLEN);
					break;
				}
				case GNS_OP_REAL:
					strncpy(brn->real, (void *) gns + offset, REALLEN);
					break;
				case GNS_OP_PRID:
					if (brn->name[0] == '\0')
						break;
					struct MeshClient *sc;
					sc = GNUNET_realloc(brn, sizeof(struct MeshClient));
					sc->remote = *(struct GNUNET_PeerIdentity *) ((void *) gns
							+ offset);
					sc->base.type = IRCD_ROUTING_NODE_MESH;
					routing_put(&sc->base, sc->base.name);
					mesh_connect(sc);
					break;
				}
			}
		}
		rd++;
	}
}

static char *label_ucase(const char *str) {
	char *label, *t;
	label = GNUNET_malloc(CHANNELLEN + 13);
	strncpy(label, str, CHANNELLEN);
	t = label;
	while ((*t = ToUpper(*t)))
	t++;
	strncat(label, ".gnunet.gns.", CHANNELLEN + 12);
	return label;
}

void gns_lookup(const struct BaseRoutingNode *reply,
		struct BaseRoutingNode *brn, const char *str) {
	if (!reply->ego)
		return;
	struct GNUNET_CRYPTO_EcdsaPublicKey zone;
	char *name;
	name = label_ucase(str);

	GNUNET_IDENTITY_ego_get_public_key(reply->ego, &zone);
	GNUNET_GNS_lookup(h, name, &zone, 6667, GNUNET_GNS_LO_DEFAULT, NULL,
			&LookupResultProcessor, brn);
	GNUNET_free(name);
}

/**
 * @brief publish gns record
 *
 * @param c nick, realname, and ecdsa key
 */
void gns_publish(struct BaseRoutingNode *brn) {
	struct GNUNET_TIME_Absolute expire;
	expire = GNUNET_TIME_absolute_add(GNUNET_TIME_absolute_get(),
			GNUNET_TIME_UNIT_DAYS);

	char *label;
	label = label_ucase(brn->name);

	const struct GNUNET_CRYPTO_EcdsaPrivateKey *key;
	key = GNUNET_IDENTITY_ego_get_private_key(brn->ego);

	size_t namelen, reallen;
	if ((namelen = strlen(brn->name)) <= NICKLEN && !IsChanPrefix(brn->name[0])) {
		GNUNET_NETWORK_STRUCT_BEGIN
		struct {
			struct GNUNET_GNSRECORD_Data rd;
			struct IrcdGnsHeader h;
			struct IrcdGnsOpcode o[3];
			struct GNUNET_CRYPTO_EcdsaPublicKey pk;
			struct GNUNET_PeerIdentity p;
			char d[NICKLEN + REALLEN + 2];
		} data;
		GNUNET_NETWORK_STRUCT_END

		reallen = strlen(brn->real);

		data.rd.data = &data.h;
		data.rd.data_size = sizeof(data.h) + sizeof(data.o);
		data.rd.data_size += sizeof(data.p) + namelen + reallen + 2;
		data.rd.expiration_time = expire.abs_value_us;
		data.rd.record_type = 6667;
		data.rd.flags = GNUNET_GNSRECORD_RF_NONE;

		data.h.type = 0;
		data.h.version = 1;
		data.h.o_count = htons(4);

		data.o[0].code = htons(GNS_OP_NICK);
		data.o[0].offset = htons(sizeof(data.pk) + sizeof(data.p));
		data.o[1].code = htons(GNS_OP_REAL);
		data.o[1].offset = htons(sizeof(data.pk) + sizeof(data.p) + namelen + 1);
		data.o[2].code = htons(GNS_OP_REPK);
		data.o[2].offset = htons(0);
		data.o[3].code = htons(GNS_OP_PRID);
		data.o[3].offset = htons(sizeof(data.pk));

		GNUNET_IDENTITY_ego_get_public_key(brn->ego, &data.pk);
		data.p = pid;
		strncpy(data.d, brn->name, NICKLEN);
		strncpy(data.d + namelen + 1, brn->real, REALLEN);

		GNUNET_NAMESTORE_records_store(ns, key, label, 1, &data.rd, NULL,
				NULL );
	} else if (brn->name[0] == '#') {
		GNUNET_NETWORK_STRUCT_BEGIN
		struct {
			struct GNUNET_GNSRECORD_Data rd;
			struct IrcdGnsHeader h;
			struct IrcdGnsOpcode o[3];
			struct GNUNET_HashCode hc;
			struct GNUNET_CRYPTO_EddsaPublicKey p;
			char d[CHANNELLEN + 1];
		} data;
		GNUNET_NETWORK_STRUCT_END

		data.rd.data = &data.h;
		data.rd.data_size = sizeof(data.h) + sizeof(data.o);
		data.rd.data_size += sizeof(data.p) + sizeof(data.hc) + namelen + 1;
		data.rd.expiration_time = expire.abs_value_us;
		data.rd.record_type = 6667;
		data.rd.flags = GNUNET_GNSRECORD_RF_NONE;

		data.h.type = 0;
		data.h.version = 1;
		data.h.o_count = htons(3);

		data.o[0].code = htons(GNS_OP_CHAN);
		data.o[0].offset = htons(sizeof(data.hc) + sizeof(data.p));
		data.o[1].code = htons(GNS_OP_HMAC);
		data.o[1].offset = htons(0);
		data.o[2].code = htons(GNS_OP_PBKY);
		data.o[2].offset = htons(sizeof(data.hc));

		// data.hc = brn->;
		// data.p = brn->;
		strcpy(data.d, brn->name);

		GNUNET_NAMESTORE_records_store(ns, key, label, 1, &data.rd, NULL,
				NULL );
	}
	GNUNET_free(label);
}

/**
 * @brief connect to namestore and gns services
 *
 * @param cfg used in connect calls and for peerid
 */
void gns_init(const struct GNUNET_CONFIGURATION_Handle *cfg) {
	GNUNET_assert( GNUNET_CRYPTO_get_peer_identity(cfg, &pid) == GNUNET_OK);
	h = GNUNET_GNS_connect(cfg);
	ns = GNUNET_NAMESTORE_connect(cfg);
}
