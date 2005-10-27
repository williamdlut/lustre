/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 *  Copyright (c) 2003 Los Alamos National Laboratory (LANL)
 *
 *   This file is part of Lustre, http://www.lustre.org/
 *
 *   Lustre is free software; you can redistribute it and/or
 *   modify it under the terms of version 2 of the GNU General Public
 *   License as published by the Free Software Foundation.
 *
 *   Lustre is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Lustre; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 *	This file implements the nal cb functions
 */


#include "gmlnd.h"

int
gmnal_recv(lnet_ni_t *ni, void *private, lnet_msg_t *lntmsg,
           int delayed, unsigned int niov, 
           struct iovec *iov, lnet_kiov_t *kiov,
           unsigned int offset, unsigned int mlen, unsigned int rlen)
{
        gmnal_ni_t      *gmni = ni->ni_data;
	gmnal_rx_t	*rx = (gmnal_rx_t*)private;
        gmnal_msg_t     *msg = GMNAL_NETBUF_MSG(&rx->rx_buf);
        int              npages = rx->rx_islarge ? gmni->gmni_large_pages : 1;
        int              payload_offset = offsetof(gmnal_msg_t, 
                                              gmm_u.immediate.gmim_payload[0]);
        int              nob = payload_offset + mlen;

	LASSERT (msg->gmm_type == GMNAL_MSG_IMMEDIATE);
        LASSERT (iov == NULL || kiov == NULL);
        
        if (rx->rx_recv_nob < nob) {
                CERROR("Short message from nid %s: got %d, need %d\n",
                       libcfs_nid2str(msg->gmm_srcnid), rx->rx_recv_nob, nob);
                gmnal_post_rx(gmni, rx);
                return -EIO;
        }

        if (kiov != NULL)
                lnet_copy_kiov2kiov(niov, kiov, offset,
                                    npages, rx->rx_buf.nb_kiov, payload_offset, 
                                    mlen);
        else
                lnet_copy_kiov2iov(niov, iov, offset,
                                   npages, rx->rx_buf.nb_kiov, payload_offset,
                                   mlen);

        lnet_finalize(ni, lntmsg, 0);
        gmnal_post_rx(gmni, rx);
	return 0;
}

int
gmnal_send(lnet_ni_t *ni, void *private, lnet_msg_t *lntmsg)
{
        lnet_hdr_t       *hdr= &lntmsg->msg_hdr;
        int               type = lntmsg->msg_type;
        lnet_process_id_t target = lntmsg->msg_target;
        int               target_is_router = lntmsg->msg_target_is_router;
        int               routing = lntmsg->msg_routing;
        unsigned int      niov = lntmsg->msg_niov;
        struct iovec     *iov = lntmsg->msg_iov;
        lnet_kiov_t      *kiov = lntmsg->msg_kiov;
        unsigned int      offset = lntmsg->msg_offset;
        unsigned int      len = lntmsg->msg_len;
	gmnal_ni_t       *gmni = ni->ni_data;
        gm_status_t       gmrc;
	gmnal_tx_t       *tx;

        LASSERT (iov == NULL || kiov == NULL);

        /* I may not block for a tx if I'm responding to an incoming message */
        tx = gmnal_get_tx(gmni);
        if (tx == NULL) {
                if (!gmni->gmni_shutdown)
                        CERROR ("Can't get tx for msg type %d for %s\n",
                                type, libcfs_nid2str(target.nid));
                return -EIO;
        }

        tx->tx_nid = target.nid;

        gmrc = gm_global_id_to_node_id(gmni->gmni_port, LNET_NIDADDR(target.nid),
                                       &tx->tx_gmlid);
        if (gmrc != GM_SUCCESS) {
                CERROR("Can't map Nid %s to a GM local ID: %d\n", 
                       libcfs_nid2str(target.nid), gmrc);
                /* NB tx_lntmsg not set => doesn't finalize */
                gmnal_tx_done(tx, -EIO);
                return -EIO;
        }

        gmnal_pack_msg(gmni, GMNAL_NETBUF_MSG(&tx->tx_buf), 
                       target.nid, GMNAL_MSG_IMMEDIATE);
        GMNAL_NETBUF_MSG(&tx->tx_buf)->gmm_u.immediate.gmim_hdr = *hdr;
        tx->tx_msgnob = offsetof(gmnal_msg_t, gmm_u.immediate.gmim_payload[0]);

        if (tx->tx_msgnob + len <= gmni->gmni_small_msgsize) {
                /* whole message fits in tx_buf */
                char *buffer = &(GMNAL_NETBUF_MSG(&tx->tx_buf)->gmm_u.immediate.gmim_payload[0]);

                if (iov != NULL)
                        lnet_copy_iov2flat(len, buffer, 0,
                                           niov, iov, offset, len);
                else
                        lnet_copy_kiov2flat(len, buffer, 0,
                                            niov, kiov, offset, len);
                
                tx->tx_msgnob += len;
                tx->tx_large_nob = 0;

                /* We've copied everything... */
                LASSERT(tx->tx_lntmsg == NULL);
                lnet_finalize(ni, lntmsg, 0);
        } else {
                /* stash payload pts to copy later */
                tx->tx_large_nob = len;
                tx->tx_large_iskiov = (kiov != NULL);
                tx->tx_large_niov = niov;
                if (tx->tx_large_iskiov)
                        tx->tx_large_frags.kiov = kiov;
                else
                        tx->tx_large_frags.iov = iov;

                /* finalize later */
                tx->tx_lntmsg = lntmsg;
        }
        
        spin_lock(&gmni->gmni_tx_lock);

        list_add_tail(&tx->tx_list, &gmni->gmni_buf_txq);
        gmnal_check_txqueues_locked(gmni);

        spin_unlock(&gmni->gmni_tx_lock);

        return 0;
}
