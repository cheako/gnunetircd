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
	if (strlen(conn->nick) != 0 && strlen(conn->user) != 0
			&& strlen(conn->host) != 0 && strlen(conn->srvr) != 0
			&& strlen(conn->real) != 0) {
		conn->commands = normal_commands;
	}
	return 0;
}

/**
 * @brief you may not reregister
 */
int registration_done_func(struct ConnectionNode *conn, int argc, char **argv) {
	GNUNET_NETWORK_socket_send(conn->nhandle,
			":gnunetircd 462 * :You may not reregister\r\n", 95 - 52);
	return 462;
}

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.1]
 */
int pass_func(struct ConnectionNode *conn, int argc, char **argv) {
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
int nick_func(struct ConnectionNode *conn, int argc, char **argv) {
	if (argc < 2) {
		GNUNET_NETWORK_socket_send(conn->nhandle,
				":gnunetircd 431 * :No nickname given\r\n", 90 - 52);
		return 461;
	}
	strncpy(conn->nick, argv[2], NICKLEN);
	if(!clean_nick_name(conn->nick))
		conn->nick[0] = '\0';
	return 0;
}

/**
 * @brief called during registration
 */
int rnick_func(struct ConnectionNode *conn, int argc, char **argv) {
	int ret = nick_func(conn, argc, argv);
	return ret == 0 ? registration_finish(conn) : ret;
}

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.3]
 */
int user_func(struct ConnectionNode *conn, int argc, char **argv) {
	if (argc < 5) {
		GNUNET_NETWORK_socket_send(conn->nhandle,
				":gnunetircd 461 * USER :Not enough parameters\r\n", 99 - 52);
		return 461;
	}
	if (!valid_username(argv[2]) || argv[5][0] == '\0') {
		GNUNET_NETWORK_socket_send(conn->nhandle,
				":gnunetircd 461 * USER :Invalid user or real name\r\n", 69 - 18);
		return 461;
	}
	strncpy(conn->user, argv[2], USERLEN);
	strncpy(conn->host, argv[3], HOSTLEN);
	strncpy(conn->srvr, argv[4], HOSTLEN);
	strncpy(conn->real, argv[5][0] == ':' ? &argv[5][1] : argv[5], REALLEN);
	return registration_finish(conn);
}
