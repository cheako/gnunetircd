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

#include "ecdsa.h"
#include <gnunet/gnunet_identity_service.h>

static struct GNUNET_IDENTITY_Handle *id;
static const struct GNUNET_CONFIGURATION_Handle *cfg;

static void Continuation(void *cls, const char *emsg);

static void EgoCallback(void *cls, const struct GNUNET_IDENTITY_Ego *ego) {
	struct BaseRoutingNode *brn = cls;
	brn->el = NULL;
	if (ego) {
		brn->ego = ego;
	} else
		GNUNET_IDENTITY_create(id, brn->real, &Continuation, brn);
}

static void Continuation(void *cls, const char *emsg) {
	struct BaseRoutingNode *brn = cls;
	if (!emsg)
		brn->el = GNUNET_IDENTITY_ego_lookup(cfg, brn->real, &EgoCallback, brn);
}

/**
 * @brief find the identity for this connection
 *
 * @param conn used for real name and the name of the identity
 */
void ecdsa_lookup(struct BaseRoutingNode *brn) {
	if (!brn->el)
		brn->el = GNUNET_IDENTITY_ego_lookup(cfg, brn->real, &EgoCallback, brn);
}

/**
 * @brief connect to identity services
 *
 * @param cfg used in connect calls
 */
void ecdsa_init(const struct GNUNET_CONFIGURATION_Handle *_cfg) {
	cfg = _cfg;
	id = GNUNET_IDENTITY_connect(cfg, NULL, NULL );
}
