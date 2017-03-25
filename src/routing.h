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

#ifndef ROUTING_H_
#define ROUTING_H_

#include <stdbool.h>
#include <stddef.h>
#include "hybrid-6/ircd_defs.h"

/**
 * @brief what follows this routing node
 */
enum IRCD_ROUTING_NODE_TYPE {

	/**
	 * @brief invalid
	 */
	IRCD_ROUTING_NODE_NONE,

	/**
	 * @brief local connection
	 */
	IRCD_ROUTING_NODE_INETD,

	/**
	 * @brief once we get the peer identity from gns
	 */
	IRCD_ROUTING_NODE_MESH,

};

/**
 * @brief closure indicating path to another node
 */
struct BaseRoutingNode {
	/**
	 * @brief what follows this struct
	 */
	enum IRCD_ROUTING_NODE_TYPE type;
	/**
	 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.2]
	 */
	char name[CHANNELLEN + 1];
	/**
	 * @brief any previous nick name
	 */
	char pnick[NICKLEN + 1];
	/**
	 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.3]
	 */
	char real[REALLEN + 1];
	/**
	 * @brief saved, never used except to prevent endless loop
	 */
	struct GNUNET_IDENTITY_EgoLookup *el;
	/**
	 * @brief used to sign gns blocks and for zone in gns lookups
	 */
	const struct GNUNET_IDENTITY_Ego *ego;
	/**
	 * @brief should the inetd close the connection
	 *
	 * set by the quit command
	 */
	bool quit;
	/**
	 * @brief state tracker
	 */
	const struct client_function *commands;
	/**
	 * @brief queued items to send
	 */
	struct WriteQueueItem {
		/**
		 * @brief CDLL
		 */
		struct WriteQueueItem *next;
		/**
		 * @brief CDLL
		 */
		struct WriteQueueItem *prev;
		/**
		 * @brief where to send response/errors.
		 */
		struct BaseRoutingNode *reply;
		/**
		 * @brief indicates length of buffer
		 */
		size_t size;
		/**
		 * @brief bytes to send to other node
		 */
		char data[];
	/**
	 * @brief write queue CDLL
	 */
	}*wqi_head;
};

void routing_put(struct BaseRoutingNode *, const char *);
struct BaseRoutingNode *routing_query(const char *);
struct BaseRoutingNode *routing_get(struct BaseRoutingNode *, const char *);
void routing_send(struct BaseRoutingNode *, struct BaseRoutingNode *,
		const char *);

void routing_init();

#endif /* ROUTING_H_ */
