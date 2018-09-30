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

/*    MEDIAN_FILTER.cpp

    This is a compiling husk of a project. Fill it in with interesting
    pixel processing code.
 
    Revision History

    Version        Change                                                    Engineer    Date
    =======        ======                                                    ========    ======
    1.0            (seemed like a good idea at the time)                    bbb            6/1/2002

    1.0            Okay, I'm leaving the version at 1.0,                    bbb            2/15/2006
                for obvious reasons; you're going to
                copy these files directly! This is the
                first XCode version, though.

    1.0            Let's simplify this barebones sample                    zal            11/11/2010

    1.0            Added new entry point                                    zal            9/18/2017

*/

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
    PF_OutData        *out_data,
    PF_ParamDef        *params[],
    PF_LayerDef        *output )
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
MedianFilter(
             PF_InData      *in_data,
             PF_OutData     *out_data,
             PF_LayerDef    *in_layer,
             PF_LayerDef    *out_layer,
             const A_long   window_size )
{
    PF_Err err = PF_Err_NONE;
    
    A_u_short r_window[window_size * window_size];
    A_u_short g_window[window_size * window_size];
    A_u_short b_window[window_size * window_size];
   
    PF_PixelType *in_pixelP  = NULL;
    PF_PixelType *out_pixelP = NULL;
    
    const A_long edgeL = window_size / 2;
    
    for (A_long x = edgeL; x < in_layer->width - edgeL; ++x) {
        for (A_long y = edgeL; y < in_layer->height - edgeL; ++y) {
            A_long i = 0;
            
            for (A_long fx = 0; fx < window_size; ++fx) {
                for (A_long fy = 0; fy < window_size; ++fy) {
                    in_pixelP  = getPixel<PF_PixelType>(in_layer,
                                                        x + fx - edgeL,
                                                        y + fy - edgeL);
                    
                    r_window[i] = in_pixelP->red;
                    g_window[i] = in_pixelP->green;
                    b_window[i] = in_pixelP->blue;
                    
                    ++i;
                }
            }
            
            out_pixelP = getPixel<PF_PixelType>(out_layer,
                                                x,
                                                y);
            
            in_pixelP  = getPixel<PF_PixelType>(in_layer,
                                                x,
                                                y);
            
            out_pixelP->red   = getMedian(r_window, window_size * window_size);
            out_pixelP->green = getMedian(g_window, window_size * window_size);
            out_pixelP->blue  = getMedian(b_window, window_size * window_size);
            out_pixelP->alpha = in_pixelP->alpha;
        }
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
    
    if (PF_WORLD_IS_DEEP(output)) {
        ERR(MedianFilter<PF_Pixel8>(in_data,
                                    out_data,
                                    &in_layer,
                                    output,
                                    filter_size));
    } else {
        ERR(MedianFilter<PF_Pixel16>(in_data,
                                     out_data,
                                     &in_layer,
                                     output,
                                     filter_size));
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

