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

#include "registration_commands.h"
#include "command_relay.h"
#include "hybrid-6/s_user.h"
#include "hybrid-6/common.h"

static int registration_finish(struct ConnectionNode *conn) {
	if (strlen(conn->base.nick) != 0 && strlen(conn->user) != 0
			&& strlen(conn->host) != 0 && strlen(conn->srvr) != 0
			&& strlen(conn->base.real) != 0) {
		conn->base.commands = normal_commands;
	}
	return 0;
}

/**
 * @brief you may not reregister
 */
int registration_done_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	routing_send(brn, routing_get(brn, brn->nick),
			":gnunetircd 462 * :You may not reregister\r\n");
	return 462;
}

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.1]
 */
int pass_func(struct BaseRoutingNode *cls, int argc, char **argv) {
	if (cls->type != IRCD_ROUTING_NODE_INETD)
		return 1;
	struct ConnectionNode *conn = (void *) cls;
	if (argc <= 0) {
		GNUNET_NETWORK_socket_send(conn->nhandle,
				":gnunetircd 461 * PASS :Not enough parameters\r\n", 99 - 52);
		return 461;
	}
	strncpy(conn->pass, argv[1], PASSWDLEN);
	return 0;
}

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.2]
 *
 * @todo handle nick collisions for local users
 */
int nick_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 2) {
		if (brn->type == IRCD_ROUTING_NODE_INETD && brn->nick[0] == '\0') {
			GNUNET_NETWORK_socket_send(((struct ConnectionNode *) brn)->nhandle,
					":gnunetircd 431 * :No nickname given\r\n", 90 - 52);
		} else {
			routing_send(brn, routing_get(brn, brn->nick),
					":gnunetircd 431 * :No nickname given\r\n");
		}
		return 461;
	}
	if (brn->nick[0]) {
		strncpy(brn->pnick, brn->nick, NICKLEN);
		routing_put(NULL, brn->pnick);
	}
	strncpy(brn->nick, argv[2], NICKLEN);
	if (!clean_nick_name(brn->nick)) {
		brn->nick[0] = '\0';
	} else
		routing_put(brn, brn->nick);
	return 0;
}

/**
 * @brief called during registration
 */
int rnick_func(struct BaseRoutingNode *cls, int argc, char **argv) {
	if (cls->type != IRCD_ROUTING_NODE_INETD)
		return 1;
	int ret = nick_func(cls, argc, argv);
	struct ConnectionNode *conn = (void *) cls;
	return ret == 0 ? registration_finish(conn) : ret;
}

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.3]
 */
int user_func(struct BaseRoutingNode *cls, int argc, char **argv) {
	if (cls->type != IRCD_ROUTING_NODE_INETD)
		return 1;
	struct ConnectionNode *conn = (void *) cls;
	if (argc < 5) {
		GNUNET_NETWORK_socket_send(conn->nhandle,
				":gnunetircd 461 * USER :Not enough parameters\r\n", 99 - 52);
		return 461;
	}
	if (!valid_username(argv[2]) || argv[5][0] == '\0') {
		GNUNET_NETWORK_socket_send(conn->nhandle,
				":gnunetircd 461 * USER :Invalid user or real name\r\n",
				69 - 18);
		return 461;
	}
	strncpy(conn->user, argv[2], USERLEN);
	strncpy(conn->host, argv[3], HOSTLEN);
	strncpy(conn->srvr, argv[4], HOSTLEN);
	strncpy(conn->base.real, argv[5][0] == ':' ? &argv[5][1] : argv[5],
			REALLEN);
	return registration_finish(conn);
}
