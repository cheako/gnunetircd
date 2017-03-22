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

#ifndef COMMAND_RELAY_H_
#define COMMAND_RELAY_H_

#include "inetd.h"

/**
 * @brief function called to process commands
 *
 * @param cls closure
 * @param argc number of arguments, length of argv
 * @param argv array of arguments
 */
typedef int (*CommandFunc)(struct ConnectionNode *cls, int argc, char **argv);

/**
 * @brief name to function mapping
 */
struct client_function {
	/**
	 * @brief command name
	 */
	char * keyword;
	/**
	 * @brief called for command
	 */
	CommandFunc function;
};

extern const struct client_function registration_commands[];

CommandFunc get_command_function(const struct client_function *, const char *);

#endif /* COMMAND_RELAY_H_ */
