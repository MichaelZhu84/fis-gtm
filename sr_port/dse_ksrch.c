/****************************************************************
 *								*
 *	Copyright 2001, 2009 Fidelity Information Services, Inc	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include "mdef.h"

#include "gtm_string.h"

#include "gdsroot.h"
#include "gtm_facility.h"
#include "fileinfo.h"
#include "gdsbt.h"
#include "gdsfhead.h"
#include "gdsblk.h"
#include "copy.h"
#include "dse.h"

/* Include prototypes */
#include "t_qread.h"
#include "mmemory.h"

GBLREF short int	patch_path_count;
GBLREF sgmnt_addrs	*cs_addrs;
GBLREF char		patch_comp_key[MAX_KEY_SZ + 1];
GBLREF unsigned char	patch_comp_count;
GBLREF bool		patch_find_root_search;
GBLDEF block_id		ksrch_root;

int dse_ksrch(block_id srch,
	      block_id_ptr_t pp,
	      int4 *off,
	      char *targ_key,
	      int targ_len)
{
	sm_uc_ptr_t	bp, b_top, rp, r_top, key_top, blk_id;
	unsigned char	cc;
	int	    	rsize;
	ssize_t		size;
	int4		cmp;
	short		dummy_short;
	int4		dummy_int;
	cache_rec_ptr_t dummy_cr;
	error_def(ERR_DSEBLKRDFAIL);

	if(!(bp = t_qread(srch, &dummy_int, &dummy_cr)))
		rts_error(VARLSTCNT(1) ERR_DSEBLKRDFAIL);
	if (((blk_hdr_ptr_t) bp)->bsiz > cs_addrs->hdr->blk_size)
		b_top = bp + cs_addrs->hdr->blk_size;
	else if (((blk_hdr_ptr_t) bp)->bsiz < SIZEOF(blk_hdr))
		b_top = bp + SIZEOF(blk_hdr);
	else
		b_top = bp + ((blk_hdr_ptr_t) bp)->bsiz;
	patch_comp_count = 0;
	patch_comp_key[0] = patch_comp_key[1] = 0;
	*off = 0;
	for (rp = bp + SIZEOF(blk_hdr); rp < b_top; rp = r_top)
	{
		*off = (int4)(rp - bp);
		GET_SHORT(dummy_short, &((rec_hdr_ptr_t)rp)->rsiz);
		rsize = dummy_short;
		if (rsize < SIZEOF(rec_hdr))
			r_top = rp + SIZEOF(rec_hdr);
		else
			r_top = rp + rsize;
		if (r_top > b_top)
			r_top = b_top;


		if (r_top - rp < (((blk_hdr_ptr_t)bp)->levl ? SIZEOF(block_id) : MIN_DATA_SIZE) + SIZEOF(rec_hdr))
		{
			*pp = 0;
			break;
		}
		for (key_top = rp + SIZEOF(rec_hdr); key_top < r_top ; )
			if (!*key_top++ && !*key_top++)
				break;

		if (((blk_hdr_ptr_t)bp)->levl && key_top > (blk_id = r_top - SIZEOF(block_id)))
			key_top = blk_id;
		if (((rec_hdr_ptr_t) rp)->cmpc > patch_comp_count)
			cc = patch_comp_count;
		else
			cc = ((rec_hdr_ptr_t) rp)->cmpc;
		size = (ssize_t)(key_top - rp - SIZEOF(rec_hdr));
		if (size > SIZEOF(patch_comp_key) - 2 - cc)
			size = SIZEOF(patch_comp_key) - 2 - cc;
		if (size < 0)
			size = 0;
		memcpy(&patch_comp_key[cc], rp + SIZEOF(rec_hdr), size);
		patch_comp_count = (int)(cc + size);
		GET_LONGP(pp, key_top);
		cmp = memvcmp(targ_key, targ_len, &patch_comp_key[0], patch_comp_count);
		if (0 > cmp)
			break;
		if (!cmp)
		{
			if (0 != ((blk_hdr_ptr_t)bp)->levl)
				break;
			if (patch_find_root_search)
			{
				for (key_top = rp + SIZEOF(rec_hdr); key_top < r_top; )
					if (!*key_top++ && !*key_top++)
						break;
				GET_LONG(ksrch_root, key_top);
			}
			return TRUE;
		}
	}
	patch_path_count++;
	if (((blk_hdr_ptr_t) bp)->levl && *pp > 0 && *pp < cs_addrs->ti->total_blks && (*pp % cs_addrs->hdr->bplmap)
	    && dse_ksrch(*pp, pp + 1, off + 1, targ_key, targ_len))
		return TRUE;
	return FALSE;

}
