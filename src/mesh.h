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

#ifndef MESH_H_
#define MESH_H_

#include <gnunet/gnunet_config.h>
#include <gnunet/platform.h>
#include <gnunet/gnunet_util_lib.h>
#include "routing.h"

struct MeshClient {
	struct BaseRoutingNode base;
	struct GNUNET_MESH_Channel *channel;
	struct GNUNET_PeerIdentity remote;
	struct GNUNET_MESH_TransmitHandle *wh;
	GNUNET_SCHEDULER_TaskIdentifier terminate_task;
	GNUNET_SCHEDULER_TaskIdentifier timeout_task;
};

void mesh_connect(struct MeshClient *);
void continue_writing(void *cls);
void mesh_init(const struct GNUNET_CONFIGURATION_Handle *);

#endif /* MESH_H_ */
