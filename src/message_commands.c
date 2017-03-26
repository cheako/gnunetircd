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

#include <gnunet/gnunet_config.h>
#include <gnunet/platform.h>
#include <gnunet/gnunet_util_lib.h>

#include "message_commands.h"

/**
 * @brief [https://tools.ietf.org/html/rfc1459#section-4.4.1]
 *
 * also used for notice, 4.4.2.
 */
int privmsg_func(struct BaseRoutingNode *brn, int argc, char **argv) {
	if (argc < 3)
		return 1;
	char *pt, *receivers, *str1, *saveptr1, *token;
	receivers = GNUNET_strdup(argv[2]);
	pt = argv[0];
	int i = 0;
	while (i < 3)
		if (*++pt == '\0') {
			*pt = ' ';
			i++;
		}
	size_t len = strlen(argv[0]) + 2;
	char *send;
	send = GNUNET_malloc(len + 1);
	strncpy(send, argv[0], len - 2);
	strncat(send, "\r\n", len);
	for (str1 = receivers;; str1 = NULL ) {
		token = strtok_r(str1, ",", &saveptr1);
		if (token == NULL || *token == '\0')
			break;
		struct BaseRoutingNode *dest;
		dest = brn->type == IRCD_ROUTING_NODE_INETD ?
				routing_get(brn, token) : routing_query(token);
		if (dest
				&& (brn->type == IRCD_ROUTING_NODE_INETD
						|| dest->type == IRCD_ROUTING_NODE_INETD))
			routing_send(brn, dest, send);
	}
	GNUNET_free(receivers);
	GNUNET_free(send);
	return 0;
}
