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

#ifndef INETD_H_
#define INETD_H_

#include <gnunet/gnunet_config.h>
#include <gnunet/platform.h>
#include <stdbool.h>
#include <gnunet/gnunet_util_lib.h>
#include "hybrid-6/ircd_defs.h"
#include "routing.h"

/**
 * @brief closure for inetd
 */
struct InetdConnection {
	struct BaseRoutingNode base;
	/**
	 * @brief incoming connection
	 */
	struct GNUNET_NETWORK_Handle *nhandle;
	/**
	 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.1]
	 */
	char pass[PASSWDLEN + 1];
	/**
	 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.3]
	 */
	char user[USERLEN + 1];
	/**
	 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.3]
	 */
	char host[HOSTLEN + 1];
	/**
	 * @brief [https://tools.ietf.org/html/rfc1459#section-4.1.3]
	 */
	char srvr[HOSTLEN + 1];
	/**
	 * @brief remaining byte(s) size
	 */
	size_t buflen;
	/**
	 * @brief remaining byte(s) from input stream
	 */
	char *buf;
};

void inetd_start_sending(void *);
void run_accept(void *, const struct GNUNET_SCHEDULER_TaskContext *);

#endif /* INETD_H_ */
