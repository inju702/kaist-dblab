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
 * Module: RDsM_ReadTrain.c
 *
 * Description:
 *  Given the ID of a train, read it from a disk into a main memory buffer
 *
 * Exports:
 *  Four RDsM_ReadTrain(PageID*, char*, Four)
 */


#include "common.h"
#include "trace.h"
#include "error.h"
#include "latch.h"
#include "RDsM.h"
#include "perProcessDS.h"
#include "perThreadDS.h"




/*
 * Function: Four RDsM_ReadTrain(PageID*, char*, Four)
 *
 * Description:
 *  Given the ID of a train, read it from a disk into a main memory buffer
 *
 * Returns:
 *  Error code
 */
Four RDsM_ReadTrain(
    Four              handle,                  /* IN    handle */
    PageID            *trainId,                /* IN identifier of the page */
    char              *bufPtr,                 /* IN a pointer for a buffer page */
    Four              sizeOfTrain)             /* IN the size of a train in the number of pages */
{
    Four              e;                       /* error returned */
    Four              entryNo;                 /* entry no of volume table entry corresponding to the given volume */
    Four              devNo;                   /* device no. in volume */
    Four              trainOffset;             /* offset of train in device (unit = # of page) */
    RDsM_VolumeInfo_T *volInfo;                /* volume information in volume table entry */


    TR_PRINT(handle, TR_RDSM, TR1, ("RDsM_ReadTrain(trainId=%P, bufPtr=%P, sizeOfTrain=%ld)",
                            trainId, bufPtr, sizeOfTrain));


    /*
     *	check input parameters
     *  Note!! unit of read must be page or train.
     */
    if (trainId == NULL || bufPtr == NULL) ERR(handle, eBADPARAMETER);
    if (sizeOfTrain != PAGESIZE2 && sizeOfTrain != TRAINSIZE2) ERR(handle, eINVALIDTRAINSIZE_RDSM);


    /*
     *	get the corresponding volume table entry via searching the volTable
     */
    e = rdsm_GetVolTableEntryNoByVolNo(handle, trainId->volNo, &entryNo);
    if (e < eNOERROR) ERR(handle, e);


    /*
     * points to the volume information
     */
    volInfo = &RDSM_VOLTABLE[entryNo].volInfo;


    /*
     *	validate the TrainId
     */
    if (trainId->volNo < 0 || trainId->pageNo < 0 || trainId->pageNo >= volInfo->numExts*volInfo->extSize)
	ERR(handle, eBADPAGEID);


    /*
     *  find device which contains train
     */
    e = rdsm_GetPhysicalInfo(handle, volInfo, trainId->pageNo, &devNo, &trainOffset);
    if (e < eNOERROR) ERR(handle, e);


    /*
     *	read the train into the buffer
     */
    e = rdsm_ReadTrain(handle, OPENFILEDESC_ARRAY(RDSM_USERVOLTABLE(handle)[entryNo].openFileDesc)[devNo], trainOffset, bufPtr, sizeOfTrain);
    if (e < eNOERROR) ERR(handle, e);


    return(eNOERROR);

} /* RDsM_ReadTrain() */