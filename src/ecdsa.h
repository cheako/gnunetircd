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

#ifndef ECDSA_H_
#define ECDSA_H_

#include <gnunet/gnunet_config.h>
#include <gnunet/platform.h>
#include <gnunet/gnunet_util_lib.h>
#include "routing.h"

void ecdsa_lookup(struct BaseRoutingNode *);
void ecdsa_init(const struct GNUNET_CONFIGURATION_Handle *);

#endif /* ECDSA_H_ */
