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

#include "misc_commands.h"

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.6.2]
 */
int ping_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	size_t n;
	char *t = NULL;

	while (t == NULL )
		t = strdup(":gnunetircd PONG gnunetircd");
	if (argc > 1) {
		n = strlen(t) + strlen(argv[2]) + 4;
		t = GNUNET_realloc(t, n);
		strncat(t, " ", n);
		strncat(t, argv[2], n);
	} else {
		n = 48 - 21 + 4;
		t = GNUNET_realloc(t, n);
	}
	strncat(t, "\r\n", n);
	if (brn->type == IRCD_ROUTING_NODE_INETD && brn->name[0] == '\0') {
		GNUNET_NETWORK_socket_send(((struct ConnectionNode *) brn)->nhandle, t,
				strlen(t));
	} else {
		routing_send(brn, routing_get(brn, brn->name), t);
	}
	GNUNET_free(t);
	return 0;
}

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.6]
 */
int quit_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (brn->type != IRCD_ROUTING_NODE_INETD)
		return 1;
	brn->quit = true;
	GNUNET_NETWORK_socket_send(((struct ConnectionNode *) brn)->nhandle,
			"ERROR :Closing Link: * (Quit: )\r\n", 81 - 48);
	return 0;
}
