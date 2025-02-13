/**
 * (C) Copyright 2019 Intel Corporation.
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
 * VOS Object/Key incarnation log
 * vos/ilog.h
 *
 * Author: Jeff Olivier <jeffrey.v.olivier@intel.com>
 */

#ifndef __ILOG_H__
#define __ILOG_H__
#include <daos_types.h>

struct ilog_id {
	/** DTX of entry */
	umem_off_t	id_tx_id;
	/** timestamp of entry */
	daos_epoch_t	id_epoch;
};

/** Opaque root for incarnation log */
struct  ilog_df {
	char	id_pad[24];
};

struct umem_instance;

enum ilog_status {
	/** Log status is not set */
	ILOG_INVALID,
	/** Log entry is visible to caller */
	ILOG_COMMITTED,
	/** Log entry is not yet visible */
	ILOG_UNCOMMITTED,
	/** Log entry can be removed */
	ILOG_REMOVED,
};

/** Near term hack to hook things up with existing DTX */
struct ilog_desc_cbs {
	/** Retrieve the status of a log entry (See enum ilog_status). On error
	 *  return error code < 0.
	 */
	int (*dc_log_status_cb)(struct umem_instance *umm, umem_off_t tx_id,
				uint32_t intent, void *args);
	void	*dc_log_status_args;
	/** Check if the log entry was created by current transaction */
	int (*dc_is_same_tx_cb)(struct umem_instance *umm, umem_off_t tx_id,
				bool *same, void *args);
	void	*dc_is_same_tx_args;
	/** Register the log entry with the transaction log */
	int (*dc_log_add_cb)(struct umem_instance *umm, umem_off_t ilog_off,
			     umem_off_t *tx_id, void *args);
	void	*dc_log_add_args;
	/** Remove the log entry from the transaction log */
	int (*dc_log_del_cb)(struct umem_instance *umm, umem_off_t ilog_off,
			     umem_off_t tx_id, void *args);
	void	*dc_log_del_args;
};

/** Globally initialize incarnation log */
int
ilog_init(void);

/** Create a new incarnation log in place
 *
 *  \param	umm[IN]		The umem instance
 *  \param	root[IN]	A pointer to the allocated root
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_create(struct umem_instance *umm, struct ilog_df *root);

/** Open an existing incarnation log in place and create a handle to
 *  access it.
 *
 *  \param	umm[IN]		The umem instance
 *  \param	root[IN]	A pointer to the allocated root
 *  \param	cbs[in]		Incarnation log transaction log callbacks
 *  \param	loh[OUT]	Returned open log handle
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_open(struct umem_instance *umm, struct ilog_df *root,
	  const struct ilog_desc_cbs *cbs, daos_handle_t *loh);

/** Close an open incarnation log handle
 *
 *  \param	loh[in]	Open handle to close
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_close(daos_handle_t loh);

/** Destroy an incarnation log
 *
 *  \param	umm[in]		The umem instance
 *  \param	cbs[in]		Incarnation log transaction log callbacks
 *  \param	root[IN]	A pointer to an initialized incarnation log
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_destroy(struct umem_instance *umm, struct ilog_desc_cbs *cbs,
	     struct ilog_df *root);

/** Logs or updates an entry in the incaration log identified by the epoch
 *  and the currently executing transaction.  If a visible creation entry
 *  exists, nothing will be logged and the function will succeed.
 *
 *  \param	loh[in]		Open log handle
 *  \param	epr[in]		Limiting range
 *  \param	epoch[in]	Epoch of update
 *  \param	punch[in]	Punch if true, update otherwise
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_update(daos_handle_t loh, const daos_epoch_range_t *epr,
	    daos_epoch_t epoch, bool punch);

/** Updates specified log entry to mark it as persistent (remove
 * the transaction identifier from the entry.   Additionally, this will
 * remove redundant entries, such as later uncommitted upates.
 *
 *  \param	loh[in]		Open log handle
 *  \param	id[in]		Identifier for log entry
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_persist(daos_handle_t loh, const struct ilog_id *id);

/** Removes an aborted entry from the incarnation log
 *
 *  \param	loh[in]		Open log handle
 *  \param	id[in]		Identifier for log entry
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_abort(daos_handle_t loh, const struct ilog_id *id);

/**
 * Remove entries in the epoch range leaving only the latest update
 *
 *  \param	loh[in]		Open log handle
 *  \param	epr[in]		Epoch range to scan
 *
 *  \return 0 on success, error code on failure, 1 if the log is empty after
 *  completion.
 */
int
ilog_aggregate(daos_handle_t loh, const daos_epoch_range_t *epr);

/** Incarnation log entry description */
struct ilog_entry {
	/** The epoch and tx_id for the log entry */
	struct ilog_id	ie_id;
	/** If true, entry is a punch, otherwise, it's a creation */
	bool		ie_punch;
	/** The status of the incarnation log entry.  See enum ilog_status */
	int32_t		ie_status;
};

#define ILOG_PRIV_SIZE 240
/** Structure for storing the full incarnation log for ilog_fetch.  The
 * fields shouldn't generally be accessed directly but via the iteration
 * APIs below.
 */
struct ilog_entries {
	/** Array of log entries */
	struct ilog_entry	*ie_entries;
	/** Number of entries in the log */
	int64_t			 ie_num_entries;
	/** Private log data */
	uint8_t			 ie_priv[ILOG_PRIV_SIZE];
};

/** Initialize an ilog_entries struct for fetch
 *
 *  \param	entries[in]	Allocated structure where entries are stored
 */
void
ilog_fetch_init(struct ilog_entries *entries);

/** Fetch the entire incarnation log.  This function will refresh only when
 * the underlying log or the intent has changed.  If the struct is shared
 * between multiple ULT's fetch should be done after every yield.
 *
 *  \param	umm[in]		The umem instance
 *  \param	root[in]	Pointer to log root
 *  \param	cbs[in]		Incarnation log transaction log callbacks
 *  \param	intent[in]	The intent of the operation
 *  \param	entries[in,out]	Allocated structure passed in will be filled
 *				with incarnation log entries in the range.
 *
 *  \return 0 on success, error code on failure
 */
int
ilog_fetch(struct umem_instance *umm, struct ilog_df *root,
	   const struct ilog_desc_cbs *cbs, uint32_t intent,
	   struct ilog_entries *entries);

/** Deallocate any memory associated with an ilog_entries struct for fetch
 *
 *  \param	entries[in]	Allocated structure to be finalized
 */
void
ilog_fetch_finish(struct ilog_entries *entries);

/** Iterator for fetched incarnation log entries
 *
 *  \param	entries[in]	The fetched entries
 */
#define ilog_foreach_entry(ents, entry)		\
	for (entry = &(ents)->ie_entries[0];	\
	     entry != &(ents)->ie_entries[(ents)->ie_num_entries]; entry++)

/** Reverse iterator for fetched incarnation log entries
 *
 *  \param	entries[in]	The fetched entries
 */
#define ilog_foreach_entry_reverse(ents, entry)				\
	for (entry = &(ents)->ie_entries[(ents)->ie_num_entries - 1];	\
	     entry != &(ents)->ie_entries[-1]; entry--)

#endif /* __ILOG_H__ */
