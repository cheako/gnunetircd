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
#include "channel_commands.h"
#include "message_commands.h"
#include "hybrid-6/irc_string.h"
#include "inetd.h"
#include "dht.h"

int it_strcmp(void *cls, const struct GNUNET_HashCode *key, void *value) {
	return strcmp(cls, value) != 0 ? GNUNET_YES : GNUNET_NO;
}

void member_add(struct DHTClient *d, char *name) {
	char *value;
	struct GNUNET_HashCode key;
	GNUNET_CRYPTO_hash(name, strlen(name), &key);
	if (GNUNET_CONTAINER_multihashmap_get_multiple(d->members, &key, it_strcmp,
			name) == GNUNET_SYSERR)
		return;
	value = GNUNET_strdup(name);
	GNUNET_CONTAINER_multihashmap_put(d->members, &key, value,
			GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE);
}

void connect_inetd2dht(struct InetdConnection *c, struct DHTClient *d) {
	struct InetdChannel *i;
	struct DHTChannel *h;
	i = GNUNET_new(struct InetdChannel);
	h = GNUNET_new(struct DHTChannel);
	i->client = d;
	i->channel = h;
	h->connection = c;
	h->channel = i;
	GNUNET_CONTAINER_CDLL_push(c->channels_head, i);
	GNUNET_CONTAINER_CDLL_push(d->connection_head, h);
}

int ot_join_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 2)
		return 1;
	char send[BUFSIZE] = "JOIN ", *str1, *saveptr1, *token;
	for (str1 = argv[2];; str1 = NULL ) {
		token = strtok_r(str1, ",", &saveptr1);
		if (token == NULL || *token == '\0')
			break;
		struct BaseRoutingNode *dest;
		dest = routing_get(brn, token);
		if (dest && dest->type == IRCD_ROUTING_NODE_DHT)
			routing_send(brn, dest, send);
	}
	GNUNET_free(send);

	// IRCD_ROUTING_NODE_DHT;
	return -1;
}

int in_join_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 2)
		return 1;
	// IRCD_ROUTING_NODE_INETD;
	return -1;
}

int ot_part_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 2)
		return 1;
	// IRCD_ROUTING_NODE_DHT;
	// GNUNET_CONTAINER_CDLL_remove(sc->base.wqi_head, wqi);
	return -1;
}

int in_part_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 2)
		return 1;
	// IRCD_ROUTING_NODE_INETD;
	return -1;
}

int ot_privmsg_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 3)
		return 1;
	// IRCD_ROUTING_NODE_DHT;
	return -1;
}

int in_privmsg_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 3)
		return 1;
	// IRCD_ROUTING_NODE_INETD;
	return -1;
}
