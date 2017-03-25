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
#include "message_commands.h"
#include "registration_commands.h"
#include "command_relay.h"

/**
 * @brief available upon connection
 */
const struct client_function registration_commands[] = {
		{ "pass", pass_func },
		{ "nick", rnick_func },
		{ "user", user_func },
		{ "ping", ping_func },
		{ "quit", quit_func },
		{ NULL, NULL } };

/**
 * @brief regular
 */
const struct client_function normal_commands[] = {
		{ "pass", registration_done_func },
		{ "nick", nick_func },
		{ "user", registration_done_func },
		{ "ping", ping_func },
		{ "quit", quit_func },
		{ "privmsg", privmsg_func },
		{ "notice", privmsg_func },
		{ NULL, NULL } };

/**
 * @brief registration
 */
const struct client_function inbound_mesh_commands[] = {
		{ "ping", ping_func },
		{ "quit", quit_func },
		{ "privmsg", privmsg_func },
		{ "notice", privmsg_func },
		{ NULL, NULL } };

CommandFunc get_command_function(const struct client_function *commands,
		const char *cmd) {

	if (cmd == NULL )
		return NULL ;

	int i;
	for (i = 0; commands[i].keyword != NULL ; i++)
		if (0 == strcasecmp(cmd, commands[i].keyword))
			return commands[i].function;

	return NULL ;
}
