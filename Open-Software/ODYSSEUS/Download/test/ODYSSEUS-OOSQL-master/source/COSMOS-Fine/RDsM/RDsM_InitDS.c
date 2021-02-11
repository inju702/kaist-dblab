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
 * Module: RDsM_InitDS.c
 *
 * Description:
 *  Initialize data structures used in Raw Disk Manager.
 *
 * Exports:
 *  Four RDsM_InitSharedDS(void)
 *  Four RDsM_InitLocalDS(void)
 */


#include "common.h"
#include "trace.h"
#include "error.h"
#include "latch.h"
#include "SHM.h"
#include "RDsM.h"
#include "Util_heap.h"
#include "Util_varArray.h"
#include "perProcessDS.h"
#include "perThreadDS.h"



/*
 * Function: Four RDsM_InitSharedDS(void)
 *
 * Description:
 *   Initialize the VolTable[] for the Raw Disk Manager.
 *
 * Returns:
 *  Error code
 */
Four RDsM_InitSharedDS(
    Four                handle 		/* handle */
)
{
    register Four       e;
    register Four	i, j;	/* loop index */
    rdsm_VolTableEntry_T *entry;/* volume table entry (in shared memory) corresponding to the given volume */


    TR_PRINT(handle, TR_RDSM, TR1, ("RDsM_InitSharedDS()"));


    SHM_initLatch(handle, &RDSM_LATCH_VOLTABLE);


    /*
     *  Initialize heap for devInfoArray & segmentInfoArray in volume information table
     */

    e = Util_initHeap(handle, &RDSM_DEVINFOTABLEHEAP, sizeof(RDsM_DevInfo), INIT_SIZE_OF_DEVINFO_ARRAY_HEAP);
    if (e < eNOERROR) ERR(handle, e);

    e = Util_initHeap(handle, &RDSM_DEVINFOFORDATAVOLTABLEHEAP, sizeof(RDsM_DevInfoForDataVol), INIT_SIZE_OF_DEVINFO_ARRAY_HEAP);
    if (e < eNOERROR) ERR(handle, e);

    e = Util_initHeap(handle, &RDSM_SEGMENTINFOTABLEHEAP, sizeof(RDsM_SegmentInfo), INIT_SIZE_OF_SEGMENTINFO_ARRAY_HEAP);
    if (e < eNOERROR) ERR(handle, e);


    /*
     *	nullify all the volume entries
     */
    entry = &RDSM_VOLTABLE[0];
    for (i = 0; i < MAXNUMOFVOLS; i++, entry++) {

	SHM_initLatch(handle, &entry->latch);

	entry->volInfo.volNo = NOVOL;
	entry->volInfo.title[0] = '\0';                   /* null string */
	entry->volInfo.numDevices = 0;
	entry->nMounts = 0;

        entry->volInfo.devInfo = LOGICAL_PTR(NULL); 
        entry->volInfo.dataVol.devInfo = LOGICAL_PTR(NULL); 

        /* Initialize pageAllocDeallocLatch which is used to page alloc & free algorithm */
    	SHM_initLatch(handle, &RDSM_LATCH_PAGEALLOCDEALLOC(&(entry->volInfo)));

    }



    return(eNOERROR);

} /* RDsM_InitSharedDS() */


/*
 * Function: Four RDsM_InitLocalDS(void)
 *
 * Description:
 *   initialize the private data structure in RDsM.
 *
 * Returns:
 *  Error code
 */
Four RDsM_InitLocalDS(
    Four handle 		/* handle */
)
{
    Four e;
    Four i, j;


    /* pointer for RDsM Data Structure of perThreadTable */
    RDsM_PerThreadDS_T *rdsm_perThreadDSptr = RDsM_PER_THREAD_DS_PTR(handle);


    for (i = 0; i < MAXNUMOFVOLS; i++) {

	e = Util_initVarArray(handle, &RDSM_USERVOLTABLE(handle)[i].openFileDesc, sizeof(FileDesc), INIT_SIZE_OF_DEVINFO_ARRAY);
        if (e < eNOERROR) ERR(handle, e);

        RDSM_USERVOLTABLE(handle)[i].volNo = NOVOL;
        RDSM_USERVOLTABLE(handle)[i].numDevices = 0;
	for (j = 0; j < INIT_SIZE_OF_DEVINFO_ARRAY; j++)
            OPENFILEDESC_ARRAY(RDSM_USERVOLTABLE(handle)[i].openFileDesc)[j] = NIL;
    }

    /* initialize aligned system read/write buffer */
#ifdef READ_WRITE_BUFFER_ALIGN_FOR_LINUX
    e = rdsm_InitReadWriteBuffer(handle, &rdsm_perThreadDSptr->rdsm_ReadWriteBuffer);
#endif


    return(eNOERROR);

} /* RDsM_InitLocalDS() */
