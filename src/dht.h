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

#ifndef DHT_H_
#define DHT_H_

#include <gnunet/gnunet_config.h>
#include <gnunet/platform.h>
#include <gnunet/gnunet_util_lib.h>
#include "routing.h"

struct DHTClient {
	struct BaseRoutingNode base;
	int keys_cnt;
	int join_index;
	struct DHTClientKey {
		struct GNUNET_HashCode key;
		struct GNUNET_TIME_Absolute exp;
		struct GNUNET_DHT_GetHandle *gh;
		struct GNUNET_DHT_PutHandle *ph;
		struct GNUNET_HashCode val_hash;
		bool channel_state;
		GNUNET_SCHEDULER_TaskIdentifier republish_task;
		char *val;
		size_t val_size;
	} *keys;
	struct GNUNET_CRYPTO_EddsaPublicKey pub;
	struct BaseRoutingNode *req;
	struct GNUNET_GNS_LookupRequest *lr;
	GNUNET_SCHEDULER_TaskIdentifier timeout_gns;
};

void dht_continue_writing(void *cls);
void dht_init(const struct GNUNET_CONFIGURATION_Handle *);

#endif /* DHT_H_ */
