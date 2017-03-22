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

#include "command_relay.h"

static void LineLocator(struct ConnectionNode *conn) {
	/* look for the end of the line */
	char *str1, *saveptr1, *ntoken, *token;
	bool endswell = false;
	if (conn->buf[conn->buflen - 1] == 13
			|| (conn->buf[conn->buflen - 1] == 10
					&& conn->buf[conn->buflen - 2] == 13))
		endswell = true;

	for (str1 = conn->buf;; str1 = NULL ) {
		ntoken = strtok_r(str1, "\r", &saveptr1);
		if (saveptr1[0] == '\n')
			saveptr1++;
		if (str1 == NULL ) {
			if (strlen(token) != 0 && (ntoken != NULL || endswell)) {
				char *str2, *saveptr2, *subtoken, **argv;
				argv = GNUNET_malloc(0);
				int argc = 0;

				if (token[0] == ':') {
					for (; token[0] != ' '; token++)
						;
					for (; token[0] == ' '; token++)
						;
				}
				char *t;
				int len;
				len = strlen(token) + 2;
				t = GNUNET_malloc(len + 1);
				strcpy(t, ": ");
				strncat(t, token, len);
				token = t;
				for (str2 = token;; str2 = NULL ) {
					subtoken = strtok_r(str2, " ", &saveptr2);
					if (subtoken == NULL )
						break;
					argv = GNUNET_realloc(argv, sizeof(void*) * (argc + 1));
					argv[argc++] = subtoken;
					if (saveptr2[0] == ':') {
						argv = GNUNET_realloc(argv, sizeof(void*) * (argc + 1));
						argv[argc++] = saveptr2;
						break;
					}
				}
				CommandFunc f;
				f = get_command_function(conn->commands, argv[1]);
				if (!f) {
					GNUNET_NETWORK_socket_send(conn->nhandle,
							":gnunetircd 421 %s %s :Unknown command.\r\n",
							55 - 14);
				} else
					f(conn, --argc, argv);
				GNUNET_free(argv);
				GNUNET_free(t);
				if (ntoken == NULL ) {
					GNUNET_free(conn->buf);
					conn->buf = NULL;
					conn->buflen = 0;
					break;
				}
			} else {
				conn->buflen = strlen(token);
				str1 = strdup(token);
				GNUNET_free(conn->buf);
				conn->buf = str1;
				break;
			}
		}
		token = ntoken;
	}
}

static void close_node(struct ConnectionNode *conn) {
	GNUNET_NETWORK_socket_close(conn->nhandle);
	if (conn->buf != NULL )
		GNUNET_free(conn->buf);
	GNUNET_free(conn);
}

static void run_recv(void *cls, const struct GNUNET_SCHEDULER_TaskContext *ctx) {
	struct ConnectionNode *conn = cls;
	char buf[1024];
	int nbytes;
	if ((nbytes = GNUNET_NETWORK_socket_recv(conn->nhandle, buf,
			sizeof(buf) - 1)) <= 0) {
		/* got error or connection closed by client */
		if (nbytes != 0)
			perror("recv()");
		/* connection closed */
		close_node(conn);
	} else {
		/* Ensure this is an ansi string */
		buf[nbytes] = '\0';
		conn->buflen += nbytes;
		if (conn->buf != NULL ) {
			char *t = GNUNET_realloc(conn->buf, conn->buflen + 1);
			strncat(t, buf, conn->buflen);
			conn->buf = t;
		} else
			conn->buf = strdup(buf);
		LineLocator(conn);

		if (conn->quit) {
			close_node(conn);
		} else
			GNUNET_SCHEDULER_add_read_net(GNUNET_TIME_UNIT_FOREVER_REL,
					conn->nhandle, &run_recv, conn);
	}
}

/**
 * @brief called on incoming connection
 */
void run_accept(void *cls, const struct GNUNET_SCHEDULER_TaskContext *ctx) {
	struct GNUNET_NETWORK_Handle *h = cls;
	GNUNET_SCHEDULER_add_read_net(GNUNET_TIME_UNIT_FOREVER_REL, h, &run_accept,
			h);
	struct GNUNET_NETWORK_Handle *new = GNUNET_NETWORK_socket_accept(h, NULL,
			NULL );
	if (new == NULL )
		return;
	struct ConnectionNode *TempNode = NULL;
	TempNode = GNUNET_new(struct ConnectionNode);
	TempNode->nhandle = new;
	TempNode->quit = false;
	TempNode->commands = registration_commands;
	TempNode->buf = NULL;
	GNUNET_SCHEDULER_add_read_net(GNUNET_TIME_UNIT_FOREVER_REL, new, &run_recv,
			TempNode);
}
