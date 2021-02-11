/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 1990-2016, KAIST                                          */
/*    All rights reserved.                                                    */
/*                                                                            */
/*    Redistribution and use in source and binary forms, with or without      */
/*    modification, are permitted provided that the following conditions      */
/*    are met:                                                                */
/*                                                                            */
/*    1. Redistributions of source code must retain the above copyright       */
/*       notice, this list of conditions and the following disclaimer.        */
/*                                                                            */
/*    2. Redistributions in binary form must reproduce the above copyright    */
/*       notice, this list of conditions and the following disclaimer in      */
/*       the documentation and/or other materials provided with the           */
/*       distribution.                                                        */
/*                                                                            */
/*    3. Neither the name of the copyright holder nor the names of its        */
/*       contributors may be used to endorse or promote products derived      */
/*       from this software without specific prior written permission.        */
/*                                                                            */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       */
/*    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          */
/*    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,    */
/*    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        */
/*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER        */
/*    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT      */
/*    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN       */
/*    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         */
/*    POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/COSMOS General-Purpose Large-Scale Object Storage System --    */
/*    Fine-Granule Locking Version                                            */
/*    Version 3.0                                                             */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: odysseus.oosql@gmail.com                                        */
/*                                                                            */
/*    Bibliography:                                                           */
/*    [1] Whang, K., Lee, J., Lee, M., Han, W., Kim, M., and Kim, J., "DB-IR  */
/*        Integration Using Tight-Coupling in the Odysseus DBMS," World Wide  */
/*        Web, Vol. 18, No. 3, pp. 491-520, May 2015.                         */
/*    [2] Whang, K., Lee, M., Lee, J., Kim, M., and Han, W., "Odysseus: a     */
/*        High-Performance ORDBMS Tightly-Coupled with IR Features," In Proc. */
/*        IEEE 21st Int'l Conf. on Data Engineering (ICDE), pp. 1104-1105     */
/*        (demo), Tokyo, Japan, April 5-8, 2005. This paper received the Best */
/*        Demonstration Award.                                                */
/*    [3] Whang, K., Park, B., Han, W., and Lee, Y., "An Inverted Index       */
/*        Storage Structure Using Subindexes and Large Objects for Tight      */
/*        Coupling of Information Retrieval with Database Management          */
/*        Systems," U.S. Patent No.6,349,308 (2002) (Appl. No. 09/250,487     */
/*        (1999)).                                                            */
/*    [4] Whang, K., Lee, J., Kim, M., Lee, M., Lee, K., Han, W., and Kim,    */
/*        J., "Tightly-Coupled Spatial Database Features in the               */
/*        Odysseus/OpenGIS DBMS for High-Performance," GeoInformatica,        */
/*        Vol. 14, No. 4, pp. 425-446, Oct. 2010.                             */
/*    [5] Whang, K., Lee, J., Kim, M., Lee, M., and Lee, K., "Odysseus: a     */
/*        High-Performance ORDBMS Tightly-Coupled with Spatial Database       */
/*        Features," In Proc. 23rd IEEE Int'l Conf. on Data Engineering       */
/*        (ICDE), pp. 1493-1494 (demo), Istanbul, Turkey, Apr. 16-20, 2007.   */
/*                                                                            */
/******************************************************************************/
/*
 * Module: sm_GetCatalogEntryFromIndexId.c
 *
 * Description:
 *  Get the object identifier of the catalog object for B+ tree file containing
 *  the given index. We look up the catalog object from SM_SYSTABLES.
 *
 * Exports:
 *  Four sm_GetCatalogEntryFromIndexId(Four, Four, IndexID *, ObjectID *)
 */


#include <string.h>
#include "common.h"
#include "error.h"
#include "trace.h"
#include "latch.h"
#include "TM.h"
#include "LM.h"
#include "OM.h"
#include "BtM.h"
#include "SM.h"
#include "SHM.h"
#include "perProcessDS.h"
#include "perThreadDS.h"



/*@================================
 * sm_GetCatalogEntryFromIndexId( )
 *================================*/
/*
 * Function: Four sm_GetCatalogEntryFromIndexId(Four, Four, IndexID *, ObjectID *)
 *
 * Description:
 *  Get the object identifier of the catalog object for B+ tree file containing
 *  the given index. We look up the catalog object from SM_SYSTABLES.
 *
 * Returns:
 *  Error code
 *    eBADINDEXED_SM
 *    eNOTFOUNDCATALOGENTRY_SM
 *    some errors caused by function calls
 *
 * Side effects:
 *  1) catalogEntry - ObjectID of the catalog entry in SM_SYSTABLES
 */
Four sm_GetCatalogEntryFromIndexId(
    Four 			handle,
    Four 			v,			/* index for the used volume on the mount table */
    IndexID 			*index,			/* IN index identifier of the given index */
    ObjectID 			*catalogEntry,		/* OUT object identifier of the catalog object in SM_SYSINDEXES */
    PhysicalIndexID 		*pIid)			/* OUT physical index identifier */ 
{
    Four 			e;			/* error number */
    BtreeCursor 		schBid;			/* a B+ tree cursor */
    KeyValue 			kval;			/* key value */
    Four 			i;
    sm_CatOverlayForSysIndexes 	sysIndexesOverlay; 	/* entry of SM_SYSINDEXES */ 


    TR_PRINT(handle, TR_SM, TR1,
	     ("sm_GetCatalogEntryFromIndexId(handle, v=%ld, index=%P, catalogEntry=%P)",
	      v, index, catalogEntry));

    /*
    ** Get the B+ tree file's FileID.
    */
    /*@ Construct kval. */
    /* Construct a key for the B+ tree on IndexID field of SM_SYSINDEXES */
    kval.len = sizeof(Two) + sizeof(Four);
    memcpy(&kval.val[0], &(index->volNo), sizeof(Two)); /* volNo is Two */
    memcpy(&kval.val[sizeof(Two)], &(index->serial), sizeof(Four)); 

    /* Get the ObjectID of the catalog object for 'index' in SM_SYSINDEXES */
    e = BtM_Fetch(handle, MY_XACT_TABLE_ENTRY(handle), &(SM_MOUNTTABLE[v].sysIndexesIndexIdIndexInfo), 
		  &(SM_MOUNTTABLE[v].sysIndexesInfo.fid), 
                  &SM_SYSIDX_INDEXID_KEYDESC, &kval, SM_EQ, &kval, SM_EQ, &schBid, NULL, NULL);
    if (e < eNOERROR) ERR(handle, e);

    /* consider temporar file */
    if (schBid.flag == CURSOR_ON) {

	/* catalogEntry is one for the Btree in SM_SYSINDEXES. */
	/*             In the past catalogEntry deonted one for the Btree in SM_SYSTABLES. */

	*catalogEntry = schBid.oid;

        if (pIid != NULL) {
	    /* Read catalog entry in SM_SYSINDEXES. */
            e = OM_ReadObject(handle, MY_XACT_TABLE_ENTRY(handle), &(SM_MOUNTTABLE[v].sysIndexesInfo.fid),
                              catalogEntry, 0, REMAINDER, (char *)&sysIndexesOverlay, NULL); 
            if (e < eNOERROR) ERR(handle, e);

            /* get physical index ID */
            MAKE_PHYSICALINDEXID(*pIid, index->volNo, sysIndexesOverlay.rootPage);
	}

    } else {
	/* Check if the index is one defined on a temporary file. */
	for (i = 0; i < SM_NUM_OF_ENTRIES_OF_SI_FOR_TMP_FILES(handle); i++)
	    if (!SM_IS_UNUSED_ENTRY_OF_SI_FOR_TMP_FILES(SM_SI_FOR_TMP_FILES(handle)[i]) &&
		EQUAL_INDEXID(*index, SM_SI_FOR_TMP_FILES(handle)[i].iid)) {

		   /* HURRY UP PATCH: we know catalogEntry is not used by the caller. */
		   /* This should be changed. */
		   /* catalogEntry = (void*)&(SM_SI_FOR_TMP_FILES(handle)[i]); */
		   catalogEntry->volNo = index->volNo;
		   catalogEntry->pageNo = NIL;
		   catalogEntry->slotNo = NIL;

           if (pIid != NULL) {
               MAKE_PHYSICALINDEXID(*pIid, index->volNo, SM_SI_FOR_TMP_FILES(handle)[i].rootPage);
           }
		   break;
	    }

	/* invalid index id */
	if (i == SM_NUM_OF_ENTRIES_OF_SI_FOR_TMP_FILES(handle)) ERR(handle, eBADINDEXID);
    }

    return(eNOERROR);

} /* sm_GetCatalogEntryFromIndexId( ) */

