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

#include "dht.h"
#include <gnunet/gnunet_dht_service.h>
#include <stdbool.h>

static struct GNUNET_DHT_Handle *handle;

#define DHT_OP_JOIN 1
#define DHT_OP_PART 2
#define DHT_OP_MESG 3
#define DHT_OP_OSIG 4 /* Chanops signature */
#define DHT_OP_ESIG 5 /* Ego     signature */

GNUNET_NETWORK_STRUCT_BEGIN
struct IrcdDhtHeader {
	unsigned char type;
	unsigned char version;
	uint16_t o_count GNUNET_PACKED;
	struct IrcdDhtOpcode {
		uint16_t code GNUNET_PACKED;
		uint16_t offset GNUNET_PACKED;
	} opcodes[];
};
GNUNET_NETWORK_STRUCT_END

static void _put(struct DHTClientNode *n, size_t size, const void *data) {
	struct GNUNET_TIME_Absolute exp;
	exp = GNUNET_TIME_relative_to_absolute(
			GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES,
					n->channel_state ? 15 : 2));
	n->exp = exp;
	n->ph = GNUNET_DHT_put(handle, n->key, 5, GNUNET_DHT_RO_NONE, 6667, size,
			data, exp,
			GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES, 2), NULL,
			NULL );
}

static void republish_task(void *cls,
		const struct GNUNET_SCHEDULER_TaskContext *tc) {
	struct DHTClientNode *n = cls;
	_put(n, n->val_size, n->val);
	if (GNUNET_SCHEDULER_NO_TASK != n->republish_task)
		GNUNET_SCHEDULER_cancel(n->republish_task);
	n->republish_task = GNUNET_SCHEDULER_add_delayed(
			GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES, 12),
			&republish_task, n);
}

static void put(struct DHTClient *c, size_t size, const void *data) {
	struct GNUNET_HashCode nkey;
	struct DHTClientNode *n;
	n = GNUNET_new(struct DHTClientNode);
	bool channel_state = false;
	GNUNET_CRYPTO_hash(data, size, &nkey);
	// c->node_table;
	n->channel_state = channel_state;
	n->key = &c->key;
	_put(n, size, data);
	if (channel_state) {
		n->val = GNUNET_malloc(size);
		memcpy(n->val, data, size);
		n->val_size = size;
		n->republish_task = GNUNET_SCHEDULER_add_delayed(
				GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES, 12),
				&republish_task, n);
	}
}

/**
 * @brief process write buffer
 */
void dht_continue_writing(void *cls) {
	struct DHTClient *c = cls;
	struct WriteQueueItem *wqi;

	if (NULL == (wqi = c->base.wqi_head))
		return;

	size_t size;

	GNUNET_NETWORK_STRUCT_BEGIN
	struct {
		struct IrcdDhtHeader h;
		struct IrcdDhtOpcode o[3];
		struct GNUNET_HashCode hc;
		struct GNUNET_CRYPTO_EddsaPublicKey p;
		char d[CHANNELLEN + 1];
	} data;
	GNUNET_NETWORK_STRUCT_END

	put(c, size, &data);
}

static void GetIterator(void *cls, struct GNUNET_TIME_Absolute exp,
		const struct GNUNET_HashCode *key,
		const struct GNUNET_PeerIdentity *get_path,
		unsigned int get_path_length,
		const struct GNUNET_PeerIdentity *put_path,
		unsigned int put_path_length, enum GNUNET_BLOCK_Type type, size_t size,
		const void *data) {
	if (type != 6667)
		return;
	struct DHTClient *c = cls;
	struct GNUNET_HashCode val;
	GNUNET_CRYPTO_hash(data, size, &val);
	const struct IrcdDhtHeader *dht = data;
	if (dht->type == 0 && dht->version <= 1) {
		uint16_t nopcodes = ntohs(dht->o_count);
		size_t hsize = sizeof(struct IrcdDhtHeader)
				+ sizeof(struct IrcdDhtOpcode) * nopcodes;
		for (int i = 0; i < nopcodes; i++) {
			int offset = hsize + ntohs(dht->opcodes[i].offset);
			switch (ntohs(dht->opcodes[i].code)) {
			case DHT_OP_JOIN:
				break;
			case DHT_OP_PART:
				break;
			case DHT_OP_MESG:
				break;
			}
		}
	}
}

void record_not_found(const char *name) {
	struct DHTClient *c;
	c = GNUNET_new(struct DHTClient);
	GNUNET_CRYPTO_hash_create_random(GNUNET_CRYPTO_QUALITY_WEAK, &c->key);
}

void follow_channel(struct DHTClient *c) {
	c->members = GNUNET_CONTAINER_multihashmap_create (32, GNUNET_NO);
	c->gh = GNUNET_DHT_get_start(handle, 6667, &c->key, 5, GNUNET_DHT_RO_NONE,
			NULL, 0, &GetIterator, c);
}

/**
 * @brief connect to dht services
 *
 * Lookup channel messages and state
 *
 * @param cfg used in connect calls
 */
void dht_init(const struct GNUNET_CONFIGURATION_Handle *cfg) {
	handle = GNUNET_DHT_connect(cfg, 400);
}
