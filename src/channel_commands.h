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

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include "routing.h"

int in_join_func(struct BaseRoutingNode *, int, char **);
int in_part_func(struct BaseRoutingNode *, int, char **);
int in_privmsg_func(struct BaseRoutingNode *, int, char **);
int ot_join_func(struct BaseRoutingNode *, int, char **);
int ot_part_func(struct BaseRoutingNode *, int, char **);
int ot_privmsg_func(struct BaseRoutingNode *, int, char **);

#endif /* CHANNEL_H_ */
