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

#include "inetd.h"
#include "ecdsa.h"
#include "gns.h"
#include "routing.h"
#include "arm.h"

static int ret = 16;

static void run (void *cls, char *const *args, const char *cfgfile,
		const struct GNUNET_CONFIGURATION_Handle *cfg) {

	ecdsa_init(cfg);
	gns_init(cfg);
	routing_init();
	arm_init(cfg);

	struct sockaddr_in sa;
	struct GNUNET_NETWORK_Handle *h;

	memset (&sa, 0, sizeof (sa));
#if HAVE_SOCKADDR_IN_SIN_LEN
	sa.sin_len = sizeof (sa);
#endif
	sa.sin_family = AF_INET;
	sa.sin_port = htons (6677);
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	h = GNUNET_NETWORK_socket_create (AF_INET, SOCK_STREAM, 0);
	GNUNET_assert (h != NULL);
	GNUNET_assert (GNUNET_OK == GNUNET_NETWORK_socket_bind
			(h, (struct sockaddr *) &sa, sizeof (sa)));
	GNUNET_NETWORK_socket_listen (h, 5);
	GNUNET_SCHEDULER_add_read_net (GNUNET_TIME_UNIT_FOREVER_REL,
			h, &run_accept, h);
	ret = 0;
}

int main (int argc, char *const *argv) {
	static const struct GNUNET_GETOPT_CommandLineOption options[] = {
		GNUNET_GETOPT_OPTION_END
	};
	return (GNUNET_OK == GNUNET_PROGRAM_run (
		argc, argv, "gnunetircd",
		gettext_noop ("ircd over gnunet"),
		options, &run, NULL)) ? ret : 1;
}
