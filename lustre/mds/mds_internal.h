/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 */

#ifndef _MDS_INTERNAL_H
#define _MDS_INTERNAL_H
static inline struct mds_obd *mds_req2mds(struct ptlrpc_request *req)
{
        return &req->rq_export->exp_obd->u.mds;
}


/* mds/mds_fs.c */
struct llog_handle *mds_log_create(struct obd_device *obd, char *name);
int mds_log_close(struct llog_handle *cathandle, struct llog_handle *loghandle);
struct llog_handle *mds_log_open(struct obd_device *obd,
                                 struct llog_cookie *logcookie);
#if 0
struct llog_handle *mds_get_catalog(struct obd_device *obd);
#endif
void mds_put_catalog(struct obd_device *obd, struct llog_handle *cathandle);


/* mds/mds_reint.c */
void mds_commit_cb(struct obd_device *, __u64 last_rcvd, void *data, int error);
int mds_finish_transno(struct mds_obd *mds, struct inode *inode, void *handle,
                       struct ptlrpc_request *req, int rc, __u32 op_data);
void mds_reconstruct_generic(struct ptlrpc_request *req);
void mds_req_from_mcd(struct ptlrpc_request *req, struct mds_client_data *mcd);

/* mds/mds_lib.c */
int mds_update_unpack(struct ptlrpc_request *, int offset,
                      struct mds_update_record *);

/* mds/mds_unlink_open.c */
int mds_open_unlink_rename(struct mds_update_record *rec,
                           struct obd_device *obd, struct dentry *dparent,
                           struct dentry *dchild, void **handle);
int mds_cleanup_orphans(struct obd_device *obd);


/* mds/mds_log.c */
int mds_llog_setup(struct obd_device *obd, struct obd_device *disk_obd,
                   int index, int count, struct llog_logid *logid);
int mds_llog_cleanup(struct obd_device *obd);
int mds_llog_origin_add(struct obd_export *exp,
                        int index,
                        struct llog_rec_hdr *rec, struct lov_stripe_md *lsm,
                        struct llog_cookie *logcookies, int numcookies);
int mds_llog_repl_cancel(struct obd_device *obd, struct lov_stripe_md *lsm,
                         int count, struct llog_cookie *cookies, int flags);
int mds_log_op_unlink(struct obd_device *obd, struct inode *inode, struct lustre_msg *repmsg,
                      int offset);

/* mds/mds_lov.c */
int mds_lov_connect(struct obd_device *obd);
int mds_get_lovtgts(struct obd_device *, int tgt_count, struct obd_uuid *);
int mds_lov_write_objids(struct obd_device *obd);
void mds_lov_update_objids(struct obd_device *obd, obd_id *ids);
int mds_lov_set_growth(struct mds_obd *mds, int count);
int mds_lov_set_nextid(struct obd_device *obd);
int mds_set_lovdesc(struct obd_device *obd, struct lov_desc *desc,
                    struct obd_uuid *uuidarray);
int mds_post_mds_lovconf(struct obd_device *obd);

/* mds/mds_open.c */
int mds_query_write_access(struct inode *inode);
int mds_open(struct mds_update_record *rec, int offset,
             struct ptlrpc_request *req, struct lustre_handle *);
int mds_pin(struct ptlrpc_request *req);
int mds_mfd_close(struct ptlrpc_request *req, struct obd_device *obd,
                  struct mds_file_data *mfd, int unlink_orphan);
int mds_close(struct ptlrpc_request *req);


/* mds/mds_fs.c */
int mds_client_add(struct obd_device *obd, struct mds_obd *mds,
                   struct mds_export_data *med, int cl_off);
int mds_client_free(struct obd_export *exp, int clear_client);
int mds_obd_create(struct obd_export *exp, struct obdo *oa,
                      struct lov_stripe_md **ea, struct obd_trans_info *oti);
int mds_obd_destroy(struct obd_export *exp, struct obdo *oa,
                    struct lov_stripe_md *ea, struct obd_trans_info *oti);

/* mds/handler.c */
extern struct lvfs_callback_ops mds_lvfs_ops;
extern int mds_iocontrol(unsigned int cmd, struct obd_export *exp,
                         int len, void *karg, void *uarg);
#ifdef __KERNEL__
void mds_pack_inode2fid(struct ll_fid *fid, struct inode *inode);
void mds_pack_inode2body(struct mds_body *body, struct inode *inode);
#endif

#endif /* _MDS_INTERNAL_H */
