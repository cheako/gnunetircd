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

#ifndef REGISTRATION_COMMANDS_H_
#define REGISTRATION_COMMANDS_H_

#include "command_relay.h"

int pass_func(struct BaseRoutingNode *, int, char **);
int nick_func(struct BaseRoutingNode *, int, char **);
int rnick_func(struct BaseRoutingNode *, int, char **);
int user_func(struct BaseRoutingNode *, int, char **);
int registration_done_func(struct BaseRoutingNode *, int, char **);

#endif /* REGISTRATION_COMMANDS_H_ */
