/*
 * (C) Copyright 2016-2019 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the Apache License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 */
/**
 * \file
 *
 * DAOS management client library. It exports the mgmt API defined in
 * daos_mgmt.h
 */

#define D_LOGFAC	DD_FAC(mgmt)

#include <daos/mgmt.h>

#include <daos/agent.h>
#include <daos/drpc_modules.h>
#include <daos/drpc.pb-c.h>
#include <daos/event.h>
#include "srv.pb-c.h"
#include "rpc.h"

struct cp_arg {
	struct dc_mgmt_sys	*sys;
	crt_rpc_t		*rpc;
};

static int
cp(tse_task_t *task, void *data)
{
	struct cp_arg	*arg = data;
	int		 rc = task->dt_result;

	if (rc)
		D_ERROR("RPC error: %d\n", rc);

	dc_mgmt_sys_detach(arg->sys);
	crt_req_decref(arg->rpc);
	return rc;
}

int
dc_mgmt_svc_rip(tse_task_t *task)
{
	daos_svc_rip_t		*args;
	struct cp_arg		 cp_arg;
	crt_endpoint_t		 svr_ep;
	crt_rpc_t		*rpc = NULL;
	crt_opcode_t		 opc;
	struct mgmt_svc_rip_in	*rip_in;
	int			 rc;

	args = dc_task_get_args(task);
	rc = dc_mgmt_sys_attach(args->grp, &cp_arg.sys);
	if (rc != 0) {
		D_ERROR("failed to attach to grp %s, rc %d.\n", args->grp, rc);
		rc = DER_INVAL;
		goto out_task;
	}

	svr_ep.ep_grp = cp_arg.sys->sy_group;
	svr_ep.ep_rank = args->rank;
	svr_ep.ep_tag = daos_rpc_tag(DAOS_REQ_MGMT, 0);
	opc = DAOS_RPC_OPCODE(MGMT_SVC_RIP, DAOS_MGMT_MODULE,
			      DAOS_MGMT_VERSION);
	rc = crt_req_create(daos_task2ctx(task), &svr_ep, opc, &rpc);
	if (rc != 0) {
		D_ERROR("crt_req_create(MGMT_SVC_RIP) failed, rc: %d.\n",
			rc);
		D_GOTO(err_grp, rc);
	}

	D_ASSERT(rpc != NULL);
	rip_in = crt_req_get(rpc);
	D_ASSERT(rip_in != NULL);

	/** fill in request buffer */
	rip_in->rip_flags = args->force;

	crt_req_addref(rpc);
	cp_arg.rpc = rpc;

	rc = tse_task_register_comp_cb(task, cp, &cp_arg, sizeof(cp_arg));
	if (rc != 0)
		D_GOTO(err_rpc, rc);

	D_DEBUG(DB_MGMT, "killing rank %u\n", args->rank);

	/** send the request */
	return daos_rpc_send(rpc, task);

err_rpc:
	crt_req_decref(rpc);
err_grp:
	dc_mgmt_sys_detach(cp_arg.sys);
out_task:
	tse_task_complete(task, rc);
	return rc;
}

int
dc_mgmt_set_params(tse_task_t *task)
{
	daos_set_params_t		*args;
	struct cp_arg			cp_arg;
	struct mgmt_params_set_in	*in;
	crt_endpoint_t			ep;
	crt_rpc_t			*rpc = NULL;
	crt_opcode_t			opc;
	int				rc;

	args = dc_task_get_args(task);
	rc = dc_mgmt_sys_attach(args->grp, &cp_arg.sys);
	if (rc != 0) {
		D_ERROR("failed to attach to grp %s, rc %d.\n", args->grp, rc);
		rc = -DER_INVAL;
		goto out_task;
	}

	ep.ep_grp = cp_arg.sys->sy_group;
	/* if rank == -1 means it will set params on all servers, which we will
	 * send it to 0 temporarily.
	 */
	ep.ep_rank = args->rank == -1 ? 0 : args->rank;
	ep.ep_tag = daos_rpc_tag(DAOS_REQ_MGMT, 0);
	opc = DAOS_RPC_OPCODE(MGMT_PARAMS_SET, DAOS_MGMT_MODULE,
			      DAOS_MGMT_VERSION);
	rc = crt_req_create(daos_task2ctx(task), &ep, opc, &rpc);
	if (rc != 0) {
		D_ERROR("crt_req_create(MGMT_SVC_RIP) failed, rc: %d.\n",
			rc);
		D_GOTO(err_grp, rc);
	}

	D_ASSERT(rpc != NULL);
	in = crt_req_get(rpc);
	D_ASSERT(in != NULL);

	/** fill in request buffer */
	in->ps_rank = args->rank;
	in->ps_key_id = args->key_id;
	in->ps_value = args->value;
	in->ps_value_extra = args->value_extra;

	crt_req_addref(rpc);
	cp_arg.rpc = rpc;

	rc = tse_task_register_comp_cb(task, cp, &cp_arg, sizeof(cp_arg));
	if (rc != 0)
		D_GOTO(err_rpc, rc);

	D_DEBUG(DB_MGMT, "set parameter %d/%u/"DF_U64".\n", args->rank,
		 args->key_id, args->value);

	/** send the request */
	return daos_rpc_send(rpc, task);

err_rpc:
	crt_req_decref(rpc);
err_grp:
	dc_mgmt_sys_detach(cp_arg.sys);
out_task:
	tse_task_complete(task, rc);
	return rc;
}

int
dc_mgmt_profile(uint64_t modules, char *path, bool start)
{
	struct dc_mgmt_sys	*sys;
	struct mgmt_profile_in	*in;
	crt_endpoint_t		ep;
	crt_rpc_t		*rpc = NULL;
	crt_opcode_t		opc;
	int			rc;

	rc = dc_mgmt_sys_attach(NULL, &sys);
	if (rc != 0) {
		D_ERROR("failed to attach to grp rc %d.\n", rc);
		return -DER_INVAL;
	}

	ep.ep_grp = sys->sy_group;
	ep.ep_rank = 0;
	ep.ep_tag = daos_rpc_tag(DAOS_REQ_MGMT, 0);
	opc = DAOS_RPC_OPCODE(MGMT_PROFILE, DAOS_MGMT_MODULE,
			      DAOS_MGMT_VERSION);
	rc = crt_req_create(daos_get_crt_ctx(), &ep, opc, &rpc);
	if (rc != 0) {
		D_ERROR("crt_req_create failed, rc: %d.\n", rc);
		D_GOTO(err_grp, rc);
	}

	D_ASSERT(rpc != NULL);
	in = crt_req_get(rpc);
	in->p_module = modules;
	in->p_path = path;
	in->p_op = start ? MGMT_PROFILE_START : MGMT_PROFILE_STOP;
	/** send the request */
	rc = daos_rpc_send_wait(rpc);
err_grp:
	D_DEBUG(DB_MGMT, "mgmt profile: rc %d\n", rc);
	dc_mgmt_sys_detach(sys);
	return rc;
}

int
dc_mgmt_add_mark(const char *mark)
{
	struct dc_mgmt_sys	*sys;
	struct mgmt_mark_in	*in;
	crt_endpoint_t		ep;
	crt_rpc_t		*rpc = NULL;
	crt_opcode_t		opc;
	int			rc;

	rc = dc_mgmt_sys_attach(NULL, &sys);
	if (rc != 0) {
		D_ERROR("failed to attach to grp rc %d.\n", rc);
		return -DER_INVAL;
	}

	ep.ep_grp = sys->sy_group;
	ep.ep_rank = 0;
	ep.ep_tag = daos_rpc_tag(DAOS_REQ_MGMT, 0);
	opc = DAOS_RPC_OPCODE(MGMT_MARK, DAOS_MGMT_MODULE,
			      DAOS_MGMT_VERSION);
	rc = crt_req_create(daos_get_crt_ctx(), &ep, opc, &rpc);
	if (rc != 0) {
		D_ERROR("crt_req_create failed, rc: %d.\n", rc);
		D_GOTO(err_grp, rc);
	}

	D_ASSERT(rpc != NULL);
	in = crt_req_get(rpc);
	in->m_mark = (char *)mark;
	/** send the request */
	rc = daos_rpc_send_wait(rpc);
err_grp:
	D_DEBUG(DB_MGMT, "mgmt mark: rc %d\n", rc);
	dc_mgmt_sys_detach(sys);
	return rc;

}

struct dc_mgmt_psr {
	d_rank_t	 rank;
	char		*uri;
};

/*
 * Get the attach info (i.e., the CaRT PSRs) for name. npsrs outputs the number
 * of elements in psrs. psrs outputs the array of struct dc_mgmt_psr objects.
 * Callers are responsible for freeing psrs using put_attach_info.
 */
static int
get_attach_info(const char *name, int *npsrs, struct dc_mgmt_psr **psrs)
{
	struct drpc		*ctx;
	Mgmt__GetAttachInfoReq	 req = MGMT__GET_ATTACH_INFO_REQ__INIT;
	Mgmt__GetAttachInfoResp	*resp;
	uint8_t			*reqb;
	size_t			 reqb_size;
	Drpc__Call		*dreq;
	Drpc__Response		*dresp;
	struct dc_mgmt_psr      *p;
	int			 i;
	int			 rc;

	D_DEBUG(DB_MGMT, "getting attach info for %s\n", name);

	/* Connect to daos_agent. */
	D_ASSERT(dc_agent_sockpath != NULL);
	ctx = drpc_connect(dc_agent_sockpath);
	if (ctx == NULL) {
		D_ERROR("failed to connect to %s\n", dc_agent_sockpath);
		rc = -DER_BADPATH;
		goto out;
	}

	/* Prepare the GetAttachInfo request. */
	req.sys = (char *)name;
	reqb_size = mgmt__get_attach_info_req__get_packed_size(&req);
	D_ALLOC(reqb, reqb_size);
	if (reqb == NULL) {
		rc = -DER_NOMEM;
		goto out_ctx;
	}
	mgmt__get_attach_info_req__pack(&req, reqb);
	dreq = drpc_call_create(ctx, DRPC_MODULE_MGMT,
				DRPC_METHOD_MGMT_GET_ATTACH_INFO);
	if (dreq == NULL) {
		rc = -DER_NOMEM;
		D_FREE(reqb);
		goto out_ctx;
	}
	dreq->body.len = reqb_size;
	dreq->body.data = reqb;

	/* Make the GetAttachInfo call and get the response. */
	rc = drpc_call(ctx, R_SYNC, dreq, &dresp);
	if (rc != 0) {
		D_ERROR("GetAttachInfo call failed: %d\n", rc);
		goto out_dreq;
	}
	if (dresp->status != DRPC__STATUS__SUCCESS) {
		D_ERROR("GetAttachInfo unsuccessful: %d\n", dresp->status);
		rc = -DER_MISC;
		goto out_dresp;
	}
	resp = mgmt__get_attach_info_resp__unpack(NULL, dresp->body.len,
						  dresp->body.data);
	if (resp == NULL) {
		D_ERROR("failed to unpack GetAttachInfo response\n");
		rc = -DER_MISC;
		goto out_dresp;
	}
	if (resp->status != 0) {
		D_ERROR("GetAttachInfo failed: %d\n", resp->status);
		rc = resp->status;
		goto out_resp;
	}

	/* Output to the caller. */
	D_ALLOC_ARRAY(p, resp->n_psrs);
	if (p == NULL) {
		rc = -DER_NOMEM;
		goto out_resp;
	}
	for (i = 0; i < resp->n_psrs; i++) {
		p[i].rank = resp->psrs[i]->rank;
		D_ASPRINTF(p[i].uri, "%s", resp->psrs[i]->uri);
		if (p[i].uri == NULL) {
			rc = -DER_NOMEM;
			break;
		}
	}
	if (rc != 0) {
		for (; i >= 0; i--) {
			if (p[i].uri != NULL)
				D_FREE(p[i].uri);
		}
		D_FREE(p);
		goto out_resp;
	}
	*npsrs = resp->n_psrs;
	*psrs = p;

out_resp:
	mgmt__get_attach_info_resp__free_unpacked(resp, NULL);
out_dresp:
	drpc_response_free(dresp);
out_dreq:
	/* This also frees reqb via dreq->body.data. */
	drpc_call_free(dreq);
out_ctx:
	drpc_close(ctx);
out:
	return rc;
}

#define SYS_BUF_MAGIC 0x98234ad3

struct psr_buf {
	uint32_t	psrb_rank;
	char		psrb_uri[CRT_ADDR_STR_MAX_LEN];
};

struct sys_buf {
	uint32_t	syb_magic;
	uint32_t	syb_npsrbs;
	char		syb_name[DAOS_SYS_NAME_MAX + 1];
	struct psr_buf	syb_psrbs[0];
};

/* Convert psrbs[npsrbs] to psrs[npsrs]. */
static int
get_attach_info_from_buf(int npsrbs, struct psr_buf *psrbs, int *npsrs,
			 struct dc_mgmt_psr **psrs)
{
	struct dc_mgmt_psr     *p;
	int			i;
	int			rc = 0;

	D_ALLOC_ARRAY(p, npsrbs);
	if (p == NULL)
		return -DER_NOMEM;

	for (i = 0; i < npsrbs; i++) {
		p[i].rank = psrbs[i].psrb_rank;
		D_ASPRINTF(p[i].uri, "%s", psrbs[i].psrb_uri);
		if (p[i].uri == NULL) {
			rc = -DER_NOMEM;
			break;
		}
	}
	if (rc != 0) {
		for (; i >= 0; i--) {
			if (p[i].uri != NULL)
				D_FREE(p[i].uri);
		}
		D_FREE(p);
		return rc;
	}

	*npsrs = npsrbs;
	*psrs = p;
	return 0;
}

static void
put_attach_info(int npsrs, struct dc_mgmt_psr *psrs)
{
	int i;

	for (i = 0; i < npsrs; i++)
		D_FREE(psrs[i].uri);
	D_FREE(psrs);
}

static int
attach_group(const char *name, bool pmixless, int npsrs,
	     struct dc_mgmt_psr *psrs, crt_group_t **groupp)
{
	crt_group_t    *group;
	int		i;
	int		rc;

	if (!pmixless) {
		rc = crt_group_attach((char *)name, &group);
		if (rc != 0) {
			D_ERROR("failed to attach to group %s: %d\n", name, rc);
			goto err;
		}
		goto out;
	}

	rc = crt_group_view_create((char *)name, &group);
	if (rc != 0) {
		D_ERROR("failed to create group %s: %d\n", name, rc);
		goto err;
	}

	for (i = 0; i < npsrs; i++) {
		rc = crt_group_primary_rank_add(daos_get_crt_ctx(), group,
						psrs[i].rank, psrs[i].uri);
		if (rc != 0) {
			D_ERROR("failed to add rank %u URI %s to group %s: "
				"%d\n", psrs[i].rank, psrs[i].uri, name, rc);
			goto err_group;
		}

		rc = crt_group_psr_set(group, psrs[i].rank);
		if (rc != 0) {
			D_ERROR("failed to set rank %u as group %s PSR: %d\n",
				psrs[i].rank, name, rc);
			goto err_group;
		}
	}

out:
	*groupp = group;
	return 0;

err_group:
	crt_group_view_destroy(group);
err:
	return rc;
}

static void
detach_group(bool server, bool pmixless, crt_group_t *group)
{
	int rc = 0;

	if (!server) {
		if (!pmixless)
			rc = crt_group_detach(group);
		else
			rc = crt_group_view_destroy(group);
	}
	D_ASSERTF(rc == 0, "%d\n", rc);
}

static int
attach(const char *name, int npsrbs, struct psr_buf *psrbs,
       struct dc_mgmt_sys **sysp)
{
	struct dc_mgmt_sys     *sys;
	crt_group_t	       *group;
	bool			pmixless = false;
	int			rc;

	D_DEBUG(DB_MGMT, "attaching to system '%s'\n", name);

	D_ALLOC_PTR(sys);
	if (sys == NULL) {
		rc = -DER_NOMEM;
		goto err;
	}
	D_INIT_LIST_HEAD(&sys->sy_link);
	rc = snprintf(sys->sy_name, sizeof(sys->sy_name), "%s", name);
	D_ASSERTF(rc >= 0, "%d\n", rc);
	if (rc >= sizeof(sys->sy_name)) {
		D_ERROR("system name %s longer than %zu bytes\n", sys->sy_name,
			sizeof(sys->sy_name) - 1);
		rc = -DER_OVERFLOW;
		goto err_sys;
	}

	group = crt_group_lookup((char *)name);
	if (group != NULL) {
		/* This is one of the servers. Skip the get_attach_info call. */
		sys->sy_server = true;
		sys->sy_group = group;
		goto out;
	}

	d_getenv_bool("DAOS_PMIXLESS", &pmixless);
	if (pmixless) {
		if (psrbs == NULL)
			rc = get_attach_info(name, &sys->sy_npsrs,
					     &sys->sy_psrs);
		else
			rc = get_attach_info_from_buf(npsrbs, psrbs,
						      &sys->sy_npsrs,
						      &sys->sy_psrs);
		if (rc != 0)
			goto err_sys;
		if (sys->sy_npsrs < 1) {
			D_ERROR(">= 1 PSRs required: %d\n", sys->sy_npsrs);
			goto err_psrs;
		}
	}

	rc = attach_group(name, pmixless, sys->sy_npsrs, sys->sy_psrs,
			  &sys->sy_group);
	if (rc != 0)
		goto err_psrs;

out:
	*sysp = sys;
	return 0;

err_psrs:
	if (pmixless)
		put_attach_info(sys->sy_npsrs, sys->sy_psrs);
err_sys:
	D_FREE(sys);
err:
	return rc;
}

static void
detach(struct dc_mgmt_sys *sys)
{
	bool pmixless = false;

	D_DEBUG(DB_MGMT, "detaching from system '%s'\n", sys->sy_name);
	D_ASSERT(d_list_empty(&sys->sy_link));
	D_ASSERTF(sys->sy_ref == 0, "%d\n", sys->sy_ref);
	d_getenv_bool("DAOS_PMIXLESS", &pmixless);
	detach_group(sys->sy_server, pmixless, sys->sy_group);
	if (!sys->sy_server && pmixless)
		put_attach_info(sys->sy_npsrs, sys->sy_psrs);
	D_FREE(sys);
}

static D_LIST_HEAD(systems);
static pthread_mutex_t systems_lock = PTHREAD_MUTEX_INITIALIZER;

static struct dc_mgmt_sys *
lookup_sys(const char *name)
{
	struct dc_mgmt_sys *sys;

	d_list_for_each_entry(sys, &systems, sy_link) {
		if (strcmp(sys->sy_name, name) == 0)
			return sys;
	}
	return NULL;
}

static int
sys_attach(const char *name, int npsrbs, struct psr_buf *psrbs,
	   struct dc_mgmt_sys **sysp)
{
	struct dc_mgmt_sys     *sys;
	int			rc = 0;

	if (name == NULL)
		name = DAOS_DEFAULT_SYS_NAME;

	D_MUTEX_LOCK(&systems_lock);

	sys = lookup_sys(name);
	if (sys != NULL)
		goto ok;

	rc = attach(name, npsrbs, psrbs, &sys);
	if (rc != 0)
		goto out_lock;

	d_list_add(&sys->sy_link, &systems);

ok:
	sys->sy_ref++;
	*sysp = sys;
out_lock:
	D_MUTEX_UNLOCK(&systems_lock);
	return rc;
}

/**
 * Attach to system \a name.
 *
 * \param[in]		name	system name
 * \param[in,out]	sys	system handle
 */
int
dc_mgmt_sys_attach(const char *name, struct dc_mgmt_sys **sysp)
{
	if (name == NULL)
		name = DAOS_DEFAULT_SYS_NAME;

	return sys_attach(name, 0 /* npsrs */, NULL /* psrs */, sysp);
}

/**
 * Detach from system \a sys.
 *
 * \param[in]	sys	system handle
 */
void
dc_mgmt_sys_detach(struct dc_mgmt_sys *sys)
{
	D_ASSERT(sys != NULL);
	D_MUTEX_LOCK(&systems_lock);
	sys->sy_ref--;
	if (sys->sy_ref == 0) {
		d_list_del_init(&sys->sy_link);
		detach(sys);
	}
	D_MUTEX_UNLOCK(&systems_lock);
}

/**
 * Encode \a sys into \a buf of capacity \a cap. If \a buf is NULL, just return
 * the number of bytes that would be required. If \a buf is not NULL and \a cap
 * is insufficient, return -DER_TRUNC.
 */
ssize_t
dc_mgmt_sys_encode(struct dc_mgmt_sys *sys, void *buf, size_t cap)
{
	struct sys_buf *sysb = buf;
	size_t		len;
	int		i;

	len = sizeof(*sysb) + sizeof(sysb->syb_psrbs[0]) * sys->sy_npsrs;

	if (sysb == NULL)
		return len;

	if (cap < len)
		return -DER_TRUNC;

	sysb->syb_magic = SYS_BUF_MAGIC;
	sysb->syb_npsrbs = sys->sy_npsrs;
	D_CASSERT(sizeof(sysb->syb_name) == sizeof(sys->sy_name));
	strncpy(sysb->syb_name, sys->sy_name, sizeof(sysb->syb_name));

	for (i = 0; i < sys->sy_npsrs; i++) {
		struct psr_buf *psrb = &sysb->syb_psrbs[i];

		psrb->psrb_rank = sys->sy_psrs[i].rank;
		strncpy(psrb->psrb_uri, sys->sy_psrs[i].uri,
			sizeof(psrb->psrb_uri) - 1);
		psrb->psrb_uri[sizeof(psrb->psrb_uri) - 1] = '\0';
	}

	return len;
}

/** Decode \a buf of length \a len. */
ssize_t
dc_mgmt_sys_decode(void *buf, size_t len, struct dc_mgmt_sys **sysp)
{
	struct sys_buf *sysb;
	bool		swap = false;
	int		i;

	if (len < sizeof(*sysb)) {
		D_ERROR("truncated sys_buf: %zu < %zu\n", len, sizeof(*sysb));
		return -DER_IO;
	}
	sysb = buf;

	if (sysb->syb_magic != SYS_BUF_MAGIC) {
		if (sysb->syb_magic == D_SWAP32(SYS_BUF_MAGIC)) {
			swap = true;
		} else {
			D_ERROR("bad sys_buf magic: %x\n", sysb->syb_magic);
			return -DER_IO;
		}
	}

	if (swap) {
		D_SWAP32S(&sysb->syb_magic);
		D_SWAP32S(&sysb->syb_npsrbs);
		/* sysb->syb_name does not need swapping. */
		/* sysb->syb_psrbs is decoded below. */
	}

	if (len <
	    sizeof(*sysb) + sizeof(sysb->syb_psrbs[0]) * sysb->syb_npsrbs) {
		D_ERROR("truncated sys_buf.syb_psrbs: %zu < %zu\n", len,
			sizeof(*sysb) +
			sizeof(sysb->syb_psrbs[0]) * sysb->syb_npsrbs);
		return -DER_IO;
	}

	for (i = 0; i < sysb->syb_npsrbs; i++) {
		struct psr_buf *psrb = &sysb->syb_psrbs[i];

		if (swap) {
			D_SWAP32S(&psrb->psrb_rank);
			/* psrb->psrb_uri does not need swapping. */
		}
	}

	return sys_attach(sysb->syb_name, sysb->syb_npsrbs, sysb->syb_psrbs,
			  sysp);
}

/**
 * Initialize management interface
 */
int
dc_mgmt_init()
{
	int rc;

	rc = daos_rpc_register(&mgmt_proto_fmt, MGMT_PROTO_CLI_COUNT,
				NULL, DAOS_MGMT_MODULE);
	if (rc != 0)
		D_ERROR("failed to register mgmt RPCs: %d\n", rc);

	return rc;
}

/**
 * Finalize management interface
 */
void
dc_mgmt_fini()
{
	daos_rpc_unregister(&mgmt_proto_fmt);
}

int dc2_mgmt_svc_rip(tse_task_t *task)
{
	return -DER_NOSYS;
}
