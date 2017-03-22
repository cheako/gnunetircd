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

/**
 * @brief closure for inetd
 */
struct ConnectionNode {
	/**
	 * @brief incoming connection
	 */
	struct GNUNET_NETWORK_Handle *nhandle;
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
	 * @brief remaining byte(s) size
	 */
	size_t buflen;
	/**
	 * @brief remaining byte(s) from input stream
	 */
	char *buf;
};

void run_accept(void *, const struct GNUNET_SCHEDULER_TaskContext *);

#endif /* INETD_H_ */
