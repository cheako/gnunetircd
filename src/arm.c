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

#include "arm.h"
#include <gnunet/gnunet_arm_service.h>

static struct GNUNET_ARM_Handle *h;

void ConnectionStatusCallback(void *cls, int connected) {
	if (connected == GNUNET_YES) {
		GNUNET_ARM_request_service_start(h, "identity",
				GNUNET_OS_INHERIT_STD_NONE, GNUNET_TIME_UNIT_FOREVER_REL, NULL,
				NULL );
		GNUNET_ARM_request_service_start(h, "namestore",
				GNUNET_OS_INHERIT_STD_NONE, GNUNET_TIME_UNIT_FOREVER_REL, NULL,
				NULL );
		GNUNET_ARM_request_service_start(h, "gns", GNUNET_OS_INHERIT_STD_NONE,
				GNUNET_TIME_UNIT_FOREVER_REL, NULL, NULL );
		GNUNET_ARM_request_service_start(h, "mesh", GNUNET_OS_INHERIT_STD_NONE,
				GNUNET_TIME_UNIT_FOREVER_REL, NULL, NULL );
	}
}

/**
 * @brief connect to arm services
 *
 * starts identity service
 *
 * @param cfg used in connect calls
 */
void arm_init(const struct GNUNET_CONFIGURATION_Handle *cfg) {
	h = GNUNET_ARM_connect(cfg, &ConnectionStatusCallback, NULL );
}
