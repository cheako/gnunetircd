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

#include "gnunet_container_lib.h"
#include "mesh.h"
#include <gnunet/gnunet_mesh_service.h>
#include "command_relay.h"
#include "routing.h"

#define IDLE_TIMEOUT GNUNET_TIME_relative_multiply (GNUNET_TIME_UNIT_MINUTES, 16)

static struct GNUNET_MESH_Handle* h;

static void timeout_mesh_task(void *cls,
		const struct GNUNET_SCHEDULER_TaskContext *tc) {
	struct MeshClient *sc = cls;
	struct GNUNET_MESH_Channel *tun;

	sc->timeout_task = GNUNET_SCHEDULER_NO_TASK;
	tun = sc->channel;
	sc->channel = NULL;
	GNUNET_MESH_channel_destroy(tun);
}

static void refresh_timeout_task(struct MeshClient *sc) {
	if (GNUNET_SCHEDULER_NO_TASK != sc->timeout_task)
		GNUNET_SCHEDULER_cancel(sc->timeout_task);
	sc->timeout_task = GNUNET_SCHEDULER_add_delayed(IDLE_TIMEOUT,
			&timeout_mesh_task, sc);
}

static void continue_reading(struct MeshClient *sc) {
	refresh_timeout_task(sc);
	GNUNET_MESH_receive_done(sc->channel);
}

void continue_writing(void *);

static size_t write_continuation(void *cls, size_t size, void *buf) {
	struct MeshClient *sc = cls;
	struct GNUNET_MESH_Channel *tun;
	struct WriteQueueItem *wqi;
	size_t ret;

	sc->wh = NULL;
	if (NULL == (wqi = sc->base.wqi_head))
		return 0;
	if ((0 == size) || (size < wqi->size)) {
		tun = sc->channel;
		sc->channel = NULL;
		GNUNET_MESH_channel_destroy(tun);
		return 0;
	}
	GNUNET_CONTAINER_CDLL_remove(sc->base.wqi_head, wqi);
	memcpy(buf, &wqi->data, ret = wqi->size);
	GNUNET_free(wqi);
	continue_writing(sc);
	return ret;
}

/**
 * @brief process write buffer
 */
void continue_writing(void *cls) {
	struct MeshClient *sc = cls;
	struct WriteQueueItem *wqi;
	struct GNUNET_MESH_Channel *tun;

	if (NULL != sc->wh)
		return; /* write already pending */
	if (NULL == sc->channel) {
		mesh_connect(sc);
		return;
	}
	if (NULL == (wqi = sc->base.wqi_head)) {
		continue_reading(sc);
		return;
	}
	sc->wh = GNUNET_MESH_notify_transmit_ready(sc->channel, GNUNET_NO,
			GNUNET_TIME_UNIT_FOREVER_REL, wqi->size, &write_continuation, sc);
	if (NULL == sc->wh) {
		tun = sc->channel;
		sc->channel = NULL;
		GNUNET_MESH_channel_destroy(tun);
		return;
	}
}

static int MessageCallback(void *cls, struct GNUNET_MESH_Channel *channel,
		void **channel_ctx, const struct GNUNET_MessageHeader *message) {
	struct MeshClient *sc = *channel_ctx;
	struct BaseRoutingNode *brn = &sc->base;
	if (message->type != htons(6667))
		return GNUNET_OK;

	uint16_t mlen;
	mlen = ntohs(message->size) - sizeof(struct GNUNET_MessageHeader);

	const struct {
		struct GNUNET_MessageHeader m;
		char d[];
	}*data = (void *) message;

	char *buf = GNUNET_malloc(mlen + 1);
	memcpy(buf, data->d, mlen);

	char *str1, *saveptr1, *ntoken, *token;
	for (str1 = buf;; str1 = NULL ) {
		ntoken = strtok_r(str1, "\r", &saveptr1);
		if (saveptr1[0] == '\n')
			saveptr1++;
		if (str1 == NULL ) {
			if (strlen(token) != 0 && (ntoken != NULL )) {
				char *str2, *saveptr2, *subtoken, **argv;
				argv = GNUNET_malloc(0);
				int argc = 0;

				for (str2 = token;; str2 = NULL ) {
					subtoken = strtok_r(str2, " ", &saveptr2);
					if (subtoken == NULL )
						break;
					argv = GNUNET_realloc(argv, sizeof(void*) * (argc + 1));
					argv[argc++] = subtoken;
					if (saveptr2[0] == ':') {
						argv = GNUNET_realloc(argv, sizeof(void*) * (argc + 1));
						argv[argc++] = saveptr2;
						break;
					}
				}
				CommandFunc f;
				f = get_command_function(brn->commands, argv[1]);
				if (!f) {
					routing_send(brn, routing_get(brn, brn->nick),
							":gnunetircd 421 %s %s :Unknown command.\r\n");
				} else
					f(brn, --argc, argv);
				GNUNET_free(argv);
			}
		}
		token = ntoken;
	}
	GNUNET_free(buf);
	continue_reading(sc);
	return GNUNET_OK;
}

static void *InboundChannelNotificationHandler(void *cls,
		struct GNUNET_MESH_Channel * channel,
		const struct GNUNET_PeerIdentity * initiator, uint32_t port,
		enum GNUNET_MESH_ChannelOption options) {
	struct MeshClient *sc;

	sc = GNUNET_new (struct MeshClient);
	sc->remote = *initiator;
	sc->base.commands = inbound_mesh_commands;
	refresh_timeout_task(sc);
	return sc;
}

static void ChannelEndHandler(void *cls,
		const struct GNUNET_MESH_Channel *channel, void *channel_ctx) {
	struct MeshClient *sc = channel_ctx;
	struct WriteQueueItem *wqi;

	if (NULL == sc)
		return;
	sc->channel = NULL;
	if (GNUNET_SCHEDULER_NO_TASK != sc->terminate_task)
		GNUNET_SCHEDULER_cancel(sc->terminate_task);
	if (GNUNET_SCHEDULER_NO_TASK != sc->timeout_task)
		GNUNET_SCHEDULER_cancel(sc->timeout_task);
	if (NULL != sc->wh)
		GNUNET_MESH_notify_transmit_ready_cancel(sc->wh);
	while (NULL != (wqi = sc->base.wqi_head)) {
		GNUNET_CONTAINER_CDLL_remove(sc->base.wqi_head, wqi);
		GNUNET_free(wqi);
	}
	GNUNET_free(sc);
}

void mesh_connect(struct MeshClient *sc) {
	sc->channel = GNUNET_MESH_channel_create(h, sc, &sc->remote, 6667,
			GNUNET_MESH_OPTION_RELIABLE);
	continue_writing(sc);
}

void mesh_init(const struct GNUNET_CONFIGURATION_Handle * cfg) {
	static const struct GNUNET_MESH_MessageHandler MessageHandlers[] = { {
			&MessageCallback, 6667, 0 }, { NULL, 0, 0 } };
	static const uint32_t ports[] = { 6667, 0 };
	h = GNUNET_MESH_connect(cfg, NULL, &InboundChannelNotificationHandler,
			&ChannelEndHandler, MessageHandlers, ports);
}
