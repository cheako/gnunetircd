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

static void hash_next(struct GNUNET_HashCode *result,
		const struct GNUNET_HashCode *last, const char *name) {
	GNUNET_CRYPTO_kdf(result, sizeof(struct GNUNET_HashCode), name,
			strlen(name), last, sizeof(struct GNUNET_HashCode));
}

static void get(struct DHTClient *, struct DHTClientKey *);
static void pad_a(struct DHTClient *c, int i) {
	int cnt;
	if ((cnt = i + 20) > c->keys_cnt) {
		c->keys = GNUNET_realloc (c->keys, cnt * sizeof (struct DHTClientKey));
		for (; c->keys_cnt < cnt; c->keys_cnt++) {
			hash_next(&c->keys[c->keys_cnt].key, &c->keys[c->keys_cnt - 1].key,
					c->base.name);
			get(c, &c->keys[i]);
		}
	}
}

static void _put(struct DHTClientKey *n, size_t size, const void *data) {
	struct GNUNET_TIME_Absolute exp;
	exp = GNUNET_TIME_relative_to_absolute(
			GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES,
					n->channel_state ? 15 : 2));
	n->exp = exp;
	n->ph = GNUNET_DHT_put(handle, &n->key, 5,
			GNUNET_DHT_RO_NONE, 6667, size, data, exp,
			GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES, 2), NULL,
			NULL);
}

static void republish_task(void *cls,
		const struct GNUNET_SCHEDULER_TaskContext *tc) {
	struct DHTClientKey *n = cls;
	_put(n, n->val_size, n->val);
	if (GNUNET_SCHEDULER_NO_TASK != n->republish_task)
		GNUNET_SCHEDULER_cancel(n->republish_task);
	n->republish_task = GNUNET_SCHEDULER_add_delayed(
			GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES, 12),
			&republish_task, n);
}

static void put(struct DHTClient *c, size_t size, const void *data) {
	bool channel_state = false;
	int i;
	do {
		i = GNUNET_CRYPTO_random_u32(GNUNET_CRYPTO_QUALITY_WEAK, c->keys_cnt);
	} while (c->keys[i].exp.abs_value_us
			!= GNUNET_TIME_UNIT_ZERO_ABS.abs_value_us);
	GNUNET_CRYPTO_hash(data, size, &c->keys[i].val_hash);
	c->keys[i].channel_state = channel_state;
	_put(&c->keys[i], size, data);
	if (channel_state) {
		c->keys[i].val = GNUNET_malloc(size);
		memcpy(c->keys[i].val, data, size);
		c->keys[i].val_size = size;
		c->keys[i].republish_task = GNUNET_SCHEDULER_add_delayed(
				GNUNET_TIME_relative_multiply(GNUNET_TIME_UNIT_MINUTES, 12),
				&republish_task, &c->keys[i]);
	}
	pad_a(c, i);
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
	int i;
	for (i = 0; i < c->keys_cnt; i++)
		if (GNUNET_CRYPTO_hash_cmp(key, &c->keys[i].key) == 0)
			break;
	GNUNET_assert(i != c->keys_cnt);
	struct GNUNET_HashCode val;
	GNUNET_CRYPTO_hash(data, size, &val);
	if (c->keys[i].exp.abs_value_us != GNUNET_TIME_UNIT_ZERO_ABS.abs_value_us
			&& GNUNET_CRYPTO_hash_cmp(&val, &c->keys[i].val_hash) == 0)
		return;
	c->keys[i].exp = exp;
	pad_a(c, i);
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

static void get(struct DHTClient *c, struct DHTClientKey *n) {
	n->gh = GNUNET_DHT_get_start(handle, 6667, &n->key, 5, GNUNET_DHT_RO_NONE,
			NULL, 0, &GetIterator, c);
}

void record_not_found(const char *name) {
	struct DHTClientKey *head;
	head = GNUNET_malloc(20 * sizeof(struct DHTClientKey));
	GNUNET_CRYPTO_hash_create_random(GNUNET_CRYPTO_QUALITY_WEAK, &head[0].key);
	for (int i = 1; i <= 20; i++)
		hash_next(&head[i].key, &head[i - 1].key, name);
}

void follow_channel(struct DHTClient *c) {

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
