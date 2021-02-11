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
/*    Coarse-Granule Locking (Volume Lock) Version                            */
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
 * Module: SM_FormatLogVolume.c
 *
 * Description:
 *  format a log volume.
 *
 * Exports:
 *  Four SM_FormatLogVolume(Four, char**, char*, Four, Two, Four*)
 */


#include <assert.h>
#include <limits.h>
#include "common.h"
#include "trace.h"
#include "error.h"
#include "RDsM_Internal.h"	
#include "BfM.h"
#include "RM.h"
#include "SM_Internal.h"	
#include "perThreadDS.h"
#include "perProcessDS.h"


Four _SM_FormatLogVolume(
    Four handle,
    Four        numDevices,            /* IN # of devices in formated volume */
    char        **devNames,            /* IN array of device names */
    char        *title,                /* IN volume title */
    Four        volId,                 /* IN volume number */
    Two         sizeOfExt,             /* IN number of pages in an extent */
    Four        *numPagesInDevice)     /* IN array of pages' number in each device */
{
    Four        e;                     /* error code */
    Four        i;                     /* index variable */
    Four        dummyVolId;            /* volume identifier */
    double      volumeSize;            /* volume size in # of pages */
    double      deviceSize;            /* device size in bytes */

    
    TR_PRINT(TR_SM, TR1, ("SM_FormatLogVolume(handle, numDevices=%lD, devNames=%P, title=%P, volId=%ld, sizeOfExt=%ld, numPagesInDevice=%P)",
                          numDevices, devNames, title, volId, sizeOfExt, numPagesInDevice));


    /*
     * Check parameters
     */
    if (numDevices <= 0 || devNames == NULL || title == NULL || volId < 0 || sizeOfExt < 0) ERR(handle, eBADPARAMETER);
    
    for (i = 0, volumeSize = 0; i < numDevices; i++) {
        deviceSize = numPagesInDevice[i]/sizeOfExt*sizeOfExt * PAGESIZE;
        if (deviceSize > MAX_RAW_DEVICE_OFFSET) ERR(handle, eBADPARAMETER); 
        
        volumeSize += numPagesInDevice[i]/sizeOfExt*sizeOfExt; 
    }
    if (volumeSize > MAX_PAGES_IN_VOLUME) ERR(handle, eBADPARAMETER); 


    /* Format the device in the RDsM level. */
    e = RDsM_Format(handle, numDevices, devNames, title, volId, sizeOfExt, numPagesInDevice);
    if (e < 0) ERR(handle, e);

    /* Mount the formated volume. */
    e = RDsM_Mount(handle, numDevices, devNames, &volId);
    if (e < 0) ERR(handle, e);

#ifdef DBLOCK
    /* acquire lock for log volume */
    e = SM_GetVolumeLock(handle, volId, L_X);
    if (e < eNOERROR) ERR(handle, e);
#endif

    /* Format the log volume. */
    e = RM_FormatLogVolume(handle, volId);
    if (e < 0) ERR(handle, e);    

    /*@
     * finalization
     */
    /* Dismount the volume in BfM level. */
    e = BfM_Dismount(handle, volId);
    if (e < 0) ERR(handle, e);

#ifdef DBLOCK
    /* release lock for log volume */
    e = SM_ReleaseVolumeLock(handle, volId);
    if (e < eNOERROR) ERR(handle, e);
#endif

    /* Dismount the volume in RDsM level. */
    e = RDsM_Dismount(handle, volId);
    if (e < 0) ERR(handle, e);
    
    return(eNOERROR);
    

} /* SM_FormatLogVolume() */