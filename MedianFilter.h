/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

#pragma once

#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

typedef unsigned char        u_char;
typedef unsigned short       u_short;
typedef unsigned short       u_int16;
typedef unsigned long        u_long;
typedef short int            int16;
#define PF_TABLE_BITS    12
#define PF_TABLE_SZ_16    4096

#define PF_DEEP_COLOR_AWARE 1    // make sure we get 16bpc pixels;

#include "AEConfig.h"

#ifdef AE_OS_WIN
    typedef unsigned short PixelType;
    #include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "MedianFilter_Strings.h"

#include <algorithm>

/* Versioning information */

#define MAJOR_VERSION    1
#define MINOR_VERSION    0
#define BUG_VERSION      0
#define STAGE_VERSION    PF_Stage_DEVELOP
#define BUILD_VERSION    1


/* Parameter defaults */

#define MEDIAN_FILTER_SIZE_MIN        1
#define MEDIAN_FILTER_SIZE_MAX        15
#define MEDIAN_FILTER_SIZE_DFLT       5

enum {
    MEDIAN_FILTER_INPUT = 0,
    MEDIAN_FILTER_SIZE,
    MEDIAN_FILTER_NUM_PARAMS
};

enum {
    SIZE_DISK_ID
};

typedef struct FilterInputData {
    PF_LayerDef in_layer;
    A_u_long window_size;
} FilterInputData;

extern "C" {
    DllExport
    PF_Err
    EffectMain(
               PF_Cmd       cmd,
               PF_InData    *in_data,
               PF_OutData   *out_data,
               PF_ParamDef  *params[],
               PF_LayerDef  *output,
               void         *extra );
}

#endif // MEDIAN_FILTER_H
