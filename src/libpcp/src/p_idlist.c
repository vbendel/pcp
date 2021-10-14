/*
 * Copyright (c) 2012-2015,2021 Red Hat.
 * Copyright (c) 1995 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

#include <ctype.h>
#include "pmapi.h"
#include "libpcp.h"
#include "internal.h"

/*
 * PDU for id list (PDU_PMNS_IDS and PDU_DESC_IDS)
 */
typedef struct {
    __pmPDUHdr   hdr;
    int		sts;      /* never used, protocol remnant */
    int		numids;
    pmID        idlist[1];
} idlist_t;

void
__pmDumpIDList(FILE *f, int numids, const pmID idlist[])
{
    int		i;
    char	strbuf[20];

    fprintf(f, "IDlist dump: numids = %d\n", numids);
    for (i = 0; i < numids; i++)
	fprintf(f, "  PMID[%d]: 0x%08x %s\n", i, idlist[i], pmIDStr_r(idlist[i], strbuf, sizeof(strbuf)));
}

/*
 * Send PDU_PMNS_IDS or PDU_DESC_IDS across socket.
 */
int
__pmSendIDList(int fd, int from, int numids, const pmID idlist[], int sts)
{
    idlist_t	*ip;
    int		need;
    int		j;
    int		lsts;
    int		type = (sts == -1) ? PDU_DESC_IDS : PDU_PMNS_IDS;

    if (pmDebugOptions.pmns) {
	fprintf(stderr, "%s\n", "__pmSendIDList");
	__pmDumpIDList(stderr, numids, idlist);
    }

    need = (int)(sizeof(idlist_t) + (numids-1) * sizeof(idlist[0]));

    if ((ip = (idlist_t *)__pmFindPDUBuf(need)) == NULL)
	return -oserror();
    ip->hdr.len = need;
    ip->hdr.type = type;
    ip->hdr.from = from;
    ip->sts = htonl(sts);
    ip->numids = htonl(numids);
    for (j = 0; j < numids; j++) {
	ip->idlist[j] = __htonpmID(idlist[j]);
    }

    lsts = __pmXmitPDU(fd, (__pmPDU *)ip);
    __pmUnpinPDUBuf(ip);
    return lsts;
}

/*
 * Decode a PDU_PMNS_IDS.
 * Assumes that we have preallocated idlist prior to this call
 * (i.e. we know how many should definitely be coming over)
 * Returns 0 on success.
 */
int
__pmDecodeIDList(__pmPDU *pdubuf, int numids, pmID idlist[], int *sts)
{
    idlist_t	*idlist_pdu;
    char	*pdu_end;
    int		nids;
    int		j;

    idlist_pdu = (idlist_t *)pdubuf;
    pdu_end = (char *)pdubuf + idlist_pdu->hdr.len;

    if (pdu_end - (char *)pdubuf < sizeof(idlist_t) - sizeof(pmID))
	return PM_ERR_IPC;
    *sts = ntohl(idlist_pdu->sts);
    nids = ntohl(idlist_pdu->numids);
    if (nids <= 0 || nids != numids || nids > idlist_pdu->hdr.len)
	return PM_ERR_IPC;
    if (nids >= (INT_MAX - sizeof(idlist_t)) / sizeof(pmID))
	return PM_ERR_IPC;
    if (sizeof(idlist_t) + (sizeof(pmID) * (nids-1)) > (size_t)(pdu_end - (char *)pdubuf))
	return PM_ERR_IPC;

    for (j = 0; j < numids; j++)
	idlist[j] = __ntohpmID(idlist_pdu->idlist[j]);

    if (pmDebugOptions.pmns) {
	fprintf(stderr, "%s\n", "__pmDecodeIDList");
	__pmDumpIDList(stderr, numids, idlist);
    }

    return 0;
}

/*
 * Decode a PDU_DESC_IDS (variant #2)
 * We do not have a preallocated idlist prior to this call.
 * Returns 0 on success.
 */
int
__pmDecodeIDList2(__pmPDU *pdubuf, int *numids, pmID **idlist)
{
    idlist_t	*idlist_pdu;
    pmID	*pmidlist;
    char	*pdu_end;
    int		nids;
    int		j;

    idlist_pdu = (idlist_t *)pdubuf;
    pdu_end = (char *)pdubuf + idlist_pdu->hdr.len;

    if (pdu_end - (char *)pdubuf < sizeof(idlist_t) - sizeof(pmID))
	return PM_ERR_IPC;
    if (ntohl(idlist_pdu->sts) != -1)
	return PM_ERR_IPC;
    nids = ntohl(idlist_pdu->numids);
    if (nids <= 0 || nids > idlist_pdu->hdr.len)
	return PM_ERR_IPC;
    if (nids >= (INT_MAX - sizeof(idlist_t)) / sizeof(pmID))
	return PM_ERR_IPC;
    if (sizeof(idlist_t) + (sizeof(pmID) * (nids-1)) > (size_t)(pdu_end - (char *)pdubuf))
	return PM_ERR_IPC;

    if ((pmidlist = malloc(nids * sizeof(pmDesc))) == NULL)
	return -oserror();
    for (j = 0; j < nids; j++)
	pmidlist[j] = __ntohpmID(idlist_pdu->idlist[j]);

    if (pmDebugOptions.pmns) {
	fprintf(stderr, "%s\n", "__pmDecodeIDList");
	__pmDumpIDList(stderr, nids, pmidlist);
    }

    *idlist = pmidlist;
    *numids = nids;
    return 0;
}
