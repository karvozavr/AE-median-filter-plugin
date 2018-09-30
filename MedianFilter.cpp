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

#include "MedianFilter.h"

static PF_Err 
About (
    PF_InData        *in_data,
    PF_OutData       *out_data,
    PF_ParamDef      *params[],
    PF_LayerDef      *output )
{
    AEGP_SuiteHandler suites(in_data->pica_basicP);
    
    suites.ANSICallbacksSuite1()->sprintf(  out_data->return_msg,
                                            "%s v%d.%d\r%s",
                                            STR(StrID_Name),
                                            MAJOR_VERSION,
                                            MINOR_VERSION,
                                            STR(StrID_Description));
    return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (
    PF_InData        *in_data,
    PF_OutData       *out_data,
    PF_ParamDef      *params[],
    PF_LayerDef      *output )
{
    out_data->my_version = PF_VERSION(MAJOR_VERSION,
                                      MINOR_VERSION,
                                      BUG_VERSION,
                                      STAGE_VERSION,
                                      BUILD_VERSION);

    out_data->out_flags =  PF_OutFlag_DEEP_COLOR_AWARE;    // just 16bpc, not 32bpc
    
    return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (
    PF_InData        *in_data,
    PF_OutData        *out_data,
    PF_ParamDef        *params[],
    PF_LayerDef        *output )
{
    PF_Err        err    = PF_Err_NONE;
    PF_ParamDef    def;

    AEFX_CLR_STRUCT(def);

    PF_ADD_FLOAT_SLIDERX(STR(StrID_Size_Param_Name),
                         MEDIAN_FILTER_SIZE_MIN,
                         MEDIAN_FILTER_SIZE_MAX,
                         MEDIAN_FILTER_SIZE_MIN,
                         MEDIAN_FILTER_SIZE_MAX,
                         MEDIAN_FILTER_SIZE_DFLT,
                         PF_Precision_INTEGER,
                         0,
                         0,
                         SIZE_DISK_ID);
    
    out_data->num_params = MEDIAN_FILTER_NUM_PARAMS;

    return err;
}

template <typename PF_PixelType>
static PF_PixelType *
getPixel(
          PF_LayerDef*  inputP,
          const A_long  x,
          const A_long  y )
{
    return (PF_PixelType *)((char *)inputP->data + (y * inputP->rowbytes) + x * sizeof(PF_PixelType));
}

template <typename T>
static T
getMedian(
          T* array,
          const A_long size )
{
    std::nth_element(array,
                     array + (size / 2),
                     array + size);
    
    return array[size / 2];
}

template <typename PF_PixelType>
static PF_Err
MedianFilterIterator(
                     void         *refcon,
                     A_long       xL,
                     A_long       yL,
                     PF_PixelType *inP,
                     PF_PixelType *outP)
{
    PF_Err err = PF_Err_NONE;
    
    FilterInputData *input_data = reinterpret_cast<FilterInputData *>(refcon);
    
    
    if (input_data){
        PF_LayerDef &in_layer = input_data->in_layer;
        A_u_long window_size = input_data->window_size;
        
        A_u_short r_window[window_size * window_size];
        A_u_short g_window[window_size * window_size];
        A_u_short b_window[window_size * window_size];
        
        const A_long edgeL = window_size / 2;
        
        A_u_long x, y;
        
        A_u_long i = 0;
        for (A_long fx = 0; fx < window_size; ++fx) {
            for (A_long fy = 0; fy < window_size; ++fy) {
                x = xL + fx - edgeL;
                y = yL + fy - edgeL;
                if (x > edgeL && x < in_layer.width - edgeL && y > edgeL && y < in_layer.height - edgeL) {
                    inP  = getPixel<PF_PixelType>(&in_layer,
                                                  x,
                                                  y);
                    
                    r_window[i] = inP->red;
                    g_window[i] = inP->green;
                    b_window[i] = inP->blue;
                    ++i;
                }
            }
        }
        
        outP->red   = getMedian(r_window, i);
        outP->green = getMedian(g_window, i);
        outP->blue  = getMedian(b_window, i);
        outP->alpha = inP->alpha;
    }
    
    return err;
}

static PF_Err
Render (
        PF_InData          *in_data,
        PF_OutData         *out_data,
        PF_ParamDef        *params[],
        PF_LayerDef        *output )
{
    PF_Err                err        = PF_Err_NONE;
    AEGP_SuiteHandler    suites(in_data->pica_basicP);
    
    /*    Put interesting code here. */
    A_u_long filter_size = static_cast<A_u_long>(params[MEDIAN_FILTER_SIZE]->u.fs_d.value);
    PF_LayerDef in_layer = params[MEDIAN_FILTER_INPUT]->u.ld;
    
    FilterInputData input_data;
    AEFX_CLR_STRUCT(input_data);
    input_data.in_layer = in_layer;
    input_data.window_size = filter_size;
    A_u_long linesL = output->extent_hint.bottom - output->extent_hint.top;
    
    if (PF_WORLD_IS_DEEP(output)) {
        ERR(suites.Iterate16Suite1()->iterate(in_data,
                                              0,
                                              linesL,
                                              &in_layer,
                                              NULL,
                                              (void*)&input_data,
                                              MedianFilterIterator<PF_Pixel16>,
                                              output));
    } else {
        ERR(suites.Iterate8Suite1()->iterate(in_data,
                                              0,
                                              linesL,
                                              &in_layer,
                                              NULL,
                                              (void*)&input_data,
                                              MedianFilterIterator<PF_Pixel8>,
                                              output));
    }
    
    return err;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction(
    PF_PluginDataPtr inPtr,
    PF_PluginDataCB  inPluginDataCallBackPtr,
    SPBasicSuite     *inSPBasicSuitePtr,
    const char       *inHostName,
    const char       *inHostVersion )
{
    PF_Err result = PF_Err_INVALID_CALLBACK;

    result = PF_REGISTER_EFFECT(
        inPtr,
        inPluginDataCallBackPtr,
        "MEDIAN_FILTER", // Name
        "ADBE MEDIAN_FILTER", // Match Name
        "Abramov plugins", // Category
        AE_RESERVED_INFO); // Reserved Info

    return result;
}


PF_Err
EffectMain(
    PF_Cmd          cmd,
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output,
    void            *extra)
{
    PF_Err        err = PF_Err_NONE;
    
    try {
        switch (cmd) {
            case PF_Cmd_ABOUT:

                err = About(in_data,
                            out_data,
                            params,
                            output);
                break;
                
            case PF_Cmd_GLOBAL_SETUP:

                err = GlobalSetup(in_data,
                                  out_data,
                                  params,
                                  output);
                break;
                
            case PF_Cmd_PARAMS_SETUP:

                err = ParamsSetup(in_data,
                                  out_data,
                                  params,
                                  output);
                break;
                
            case PF_Cmd_RENDER:

                err = Render(in_data,
                             out_data,
                             params,
                             output);
                break;
        }
    }
    catch(PF_Err &thrown_err){
        err = thrown_err;
    }
    return err;
}

