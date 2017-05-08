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

#include "gnunet_container_lib.h"
#include "inetd.h"
#include "gns.h"
#include "mesh.h"
#include "dht.h"
#include "ecdsa.h"
#include "hybrid-6/irc_string.h"

static struct GNUNET_CONTAINER_MultiHashMap* table;

static const struct {
	enum IRCD_ROUTING_NODE_TYPE t;
	void (*f)(void *);
} handler[] = { { IRCD_ROUTING_NODE_INETD, &inetd_start_sending }, {
		IRCD_ROUTING_NODE_MESH, &mesh_continue_writing }, {
		IRCD_ROUTING_NODE_DHT, &dht_continue_writing }, {
		IRCD_ROUTING_NODE_NONE, NULL } };

static void routing_hash(const char *name, struct GNUNET_HashCode *result) {
	char upper[CHANNELLEN + 1], *str;
	strncpy(upper, name, sizeof(upper));
	str = upper;
	while ((*str = ToUpper(*str)))
	str++;
	GNUNET_CRYPTO_hash(upper, strlen(upper), result);
}

/**
 * @brief insert new nick/chan into map
 *
 * @param value where to access destination
 * @param name for nick/channel
 */
void routing_put(struct BaseRoutingNode *value, const char *name) {
	struct GNUNET_HashCode result;
	routing_hash(name, &result);
	GNUNET_CONTAINER_multihashmap_put(table, &result, value,
			GNUNET_CONTAINER_MULTIHASHMAPOPTION_REPLACE);
}

/**
 * @brief used in privmsg
 *
 * @param label destination string
 */
struct BaseRoutingNode *routing_query(const char *label) {
	struct GNUNET_HashCode result;
	routing_hash(label, &result);
	return GNUNET_CONTAINER_multihashmap_get(table, &result);
}

/**
 * @brief create if needed
 *
 * @param reply used for gns public key
 * @param label destination string
 */
struct BaseRoutingNode *routing_get(struct BaseRoutingNode *reply,
		const char *label) {
	struct BaseRoutingNode *brn;
	brn = routing_query(label);

	if (brn == NULL ) {
		brn = GNUNET_new(struct BaseRoutingNode);
		brn->type = IRCD_ROUTING_NODE_NONE;
		brn->quit = false;
		brn->wqi_head = NULL;
		routing_put(brn, label);
		gns_lookup(reply, brn, label);
	}
	return brn;
}

/**
 * @brief add data to queue
 *
 * @param reply closure for sending errors and replies
 * @param brn destination
 * @param data string to send
 */
void routing_send(struct BaseRoutingNode *reply, struct BaseRoutingNode *brn,
		const char *data) {
	if (!brn)
		return;
	struct WriteQueueItem *wqi;
	int size;
	size = strlen(data);
	wqi = GNUNET_malloc(sizeof(struct WriteQueueItem) + size + 1);

	wqi->reply = reply;
	wqi->size = size;
	memcpy(wqi->data, data, size);

	GNUNET_CONTAINER_CDLL_push(brn->wqi_head, wqi);

	int i;
	for (i = 0; handler[i].t != IRCD_ROUTING_NODE_NONE; i++)
		if (brn->type == handler[i].t && handler[i].f)
			handler[i].f(brn);
}

/**
 * @brief setup hashmap
 */
void routing_init() {
	table = GNUNET_CONTAINER_multihashmap_create(32, GNUNET_NO);
}
