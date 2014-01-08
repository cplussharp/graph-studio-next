//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : cplussharp
//  Originally seen in http://sourceforge.net/projects/h264bitstream/
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "H264StructReader.h"

#pragma warning(disable: 4800)	// disables the warning

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

// 7.3.2.11 RBSP trailing bits syntax
void CH264StructReader::ReadTrailingBits(CBitStreamReader& bs)
{
    bs.SkipU1(); // rbsp_stop_one_bit
    while (!bs.ByteAligned())
        bs.SkipU1(); // rbsp_alignment_zero_bit
}

bool CH264StructReader::MoreRbspData(CBitStreamReader& bs) 
{
    if (bs.IsEnd()) return false;
    if (bs.PeekU1()) return false; // if next bit is 1, we've reached the stop bit
    return true;
}

void CH264StructReader::ReadVUI(CBitStreamReader& bs, vui_t& vui)
{
    memset(&vui,0,sizeof(vui_t));

    vui.aspect_ratio_info_present_flag = bs.ReadU1();
    if (vui.aspect_ratio_info_present_flag)
    {
        vui.aspect_ratio_idc = bs.ReadU8();
        if (vui.aspect_ratio_idc == 255) // Extended_SAR
        {
            vui.sar_width = bs.ReadU(16);
            vui.sar_height = bs.ReadU(16);
        }
    }

    vui.overscan_info_present_flag = bs.ReadU1();
    if (vui.overscan_info_present_flag)
        vui.overscan_appropriate_flag = bs.ReadU1();

    vui.video_signal_type_present_flag = bs.ReadU1();
    if (vui.video_signal_type_present_flag)
    {
        vui.video_format = bs.ReadU(3);
        vui.video_full_range_flag = bs.ReadU1();
        vui.colour_description_present_flag = bs.ReadU1();
        if (vui.colour_description_present_flag)
        {
            vui.colour_primaries = bs.ReadU8();
            vui.transfer_characteristics = bs.ReadU8();
            vui.matrix_coefficients = bs.ReadU8();
        }
    }

    vui.chroma_loc_info_present_flag = bs.ReadU1();
    if (vui.chroma_loc_info_present_flag)
    {
        vui.chroma_sample_loc_type_top_field = bs.ReadUE();
        vui.chroma_sample_loc_type_bottom_field = bs.ReadUE();
    }
    
    vui.timing_info_present_flag = bs.ReadU1();
    if (vui.timing_info_present_flag)
    {
        vui.num_units_in_tick = bs.ReadU32();
        vui.time_scale = bs.ReadU32();
        vui.fixed_frame_rate_flag = bs.ReadU1();
    }
    
    vui.nal_hrd_parameters_present_flag = bs.ReadU1();
    if (vui.nal_hrd_parameters_present_flag)
        ReadHRD(bs, vui.nal_hrd_parameters);

    vui.vcl_hrd_parameters_present_flag = bs.ReadU1();
    if (vui.vcl_hrd_parameters_present_flag)
        ReadHRD(bs, vui.vcl_hrd_parameters);

    if (vui.nal_hrd_parameters_present_flag || vui.vcl_hrd_parameters_present_flag)
        vui.low_delay_hrd_flag = bs.ReadU1();

    vui.pic_struct_present_flag = bs.ReadU1();

    vui.bitstream_restriction_flag = bs.ReadU1();
    if (vui.bitstream_restriction_flag)
    {
        vui.motion_vectors_over_pic_boundaries_flag = bs.ReadU1();
        vui.max_bytes_per_pic_denom = bs.ReadUE();
        vui.max_bits_per_mb_denom = bs.ReadUE();
        vui.log2_max_mv_length_horizontal = bs.ReadUE();
        vui.log2_max_mv_length_vertical = bs.ReadUE();
        vui.num_reorder_frames = bs.ReadUE();
        vui.max_dec_frame_buffering = bs.ReadUE();
    }
}

void CH264StructReader::ReadHRD(CBitStreamReader& bs, hrd_t& hrd)
{
    hrd.cpb_cnt_minus1 = bs.ReadUE();
    hrd.bit_rate_scale = bs.ReadU(4);
    hrd.cpb_size_scale = bs.ReadU(4);
    for (int i = 0; i <= hrd.cpb_cnt_minus1; i++ )
    {
        hrd.bit_rate_value_minus1[i] = bs.ReadUE();
        hrd.cpb_size_value_minus1[i] = bs.ReadUE();
        hrd.cbr_flag[i] = bs.ReadU1();
    }
    hrd.initial_cpb_removal_delay_length_minus1 = bs.ReadU(5);
    hrd.cpb_removal_delay_length_minus1 = bs.ReadU(5);
    hrd.dpb_output_delay_length_minus1 = bs.ReadU(5);
    hrd.time_offset_length = bs.ReadU(5);
}

// 7.3.2.1.1 Scaling list syntax
void CH264StructReader::ReadScalingList(CBitStreamReader& bs, int* scalingList, int sizeOfScalingList, bool* useDefaultScalingMatrixFlag)
{
    // NOTE need to be able to set useDefaultScalingMatrixFlag when reading, hence passing as pointer
    int lastScale = 8;
    int nextScale = 8;
    int delta_scale;

    for (int j=0; j<sizeOfScalingList; j++)
    {
        if (nextScale != 0)
        {
            delta_scale = bs.ReadSE();
            nextScale = ( lastScale + delta_scale + 256 ) % 256;
            *useDefaultScalingMatrixFlag = (j==0 && nextScale==0);
        }

        scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[j];
    }
}

void CH264StructReader::ReadSPS(CBitStreamReader& bs, sps_t& sps)
{
    memset(&sps,0,sizeof(sps_t));
    sps.chroma_format_idc = 1;

    sps.profile_idc = bs.ReadU8();
    sps.constraint_set0_flag = bs.ReadU1();
    sps.constraint_set1_flag = bs.ReadU1();
    sps.constraint_set2_flag = bs.ReadU1();
    sps.constraint_set3_flag = bs.ReadU1();
    sps.constraint_set4_flag = bs.ReadU1();
    sps.constraint_set5_flag = bs.ReadU1();
    /* reserved_zero_2bits */ bs.SkipU(2);
    sps.level_idc = bs.ReadU8();
    sps.seq_parameter_set_id = bs.ReadUE();

    if (sps.profile_idc == 100 || sps.profile_idc == 110 ||
        sps.profile_idc == 122 || sps.profile_idc == 144)
    {
        sps.chroma_format_idc = bs.ReadUE();
        if (sps.chroma_format_idc == 3)
            sps.residual_colour_transform_flag = bs.ReadU1();

        sps.bit_depth_luma_minus8 = bs.ReadUE();
        sps.bit_depth_chroma_minus8 = bs.ReadUE();
        sps.qpprime_y_zero_transform_bypass_flag = bs.ReadU1();
        
        sps.seq_scaling_matrix_present_flag = bs.ReadU1();
        if (sps.seq_scaling_matrix_present_flag)
        {
            for (int i=0; i<8; i++)
            {
                sps.seq_scaling_list_present_flag[i] = bs.ReadU1();
                if (sps.seq_scaling_list_present_flag[i])
                {
                    if (i<6)
                        ReadScalingList(bs, sps.ScalingList4x4[i], 16, &(sps.UseDefaultScalingMatrix4x4Flag[i]));
                    else
                        ReadScalingList(bs, sps.ScalingList8x8[i-6], 64, &(sps.UseDefaultScalingMatrix8x8Flag[i-6]));
                }
            }
        }
    }

    sps.log2_max_frame_num_minus4 = bs.ReadUE();
    
    sps.pic_order_cnt_type = bs.ReadUE();
    if (sps.pic_order_cnt_type == 0)
        sps.log2_max_pic_order_cnt_lsb_minus4 = bs.ReadUE();
    else if (sps.pic_order_cnt_type == 1)
    {
        sps.delta_pic_order_always_zero_flag = bs.ReadU1();
        sps.offset_for_non_ref_pic = bs.ReadSE();
        sps.offset_for_top_to_bottom_field = bs.ReadSE();
        sps.num_ref_frames_in_pic_order_cnt_cycle = bs.ReadUE();
        for (int i = 0; i<sps.num_ref_frames_in_pic_order_cnt_cycle; i++)
            sps.offset_for_ref_frame[ i ] = bs.ReadSE();
    }

    sps.num_ref_frames = bs.ReadUE();
    sps.gaps_in_frame_num_value_allowed_flag = bs.ReadU1();
    sps.pic_width_in_mbs_minus1 = bs.ReadUE();
    sps.pic_height_in_map_units_minus1 = bs.ReadUE();
    
    sps.frame_mbs_only_flag = bs.ReadU1();
    if (!sps.frame_mbs_only_flag)
        sps.mb_adaptive_frame_field_flag = bs.ReadU1();

    sps.direct_8x8_inference_flag = bs.ReadU1();
    
    sps.frame_cropping_flag = bs.ReadU1();
    if (sps.frame_cropping_flag)
    {
        sps.frame_crop_left_offset = bs.ReadUE();
        sps.frame_crop_right_offset = bs.ReadUE();
        sps.frame_crop_top_offset = bs.ReadUE();
        sps.frame_crop_bottom_offset = bs.ReadUE();
    }

    sps.vui_parameters_present_flag = bs.ReadU1();
    if (sps.vui_parameters_present_flag)
        ReadVUI(bs, sps.vui);

    ReadTrailingBits(bs);
}

/** 
 Calculate the log base 2 of the argument, rounded up. 
 Zero or negative arguments return zero 
 Idea from http://www.southwindsgames.com/blog/2009/01/19/fast-integer-log2-function-in-cc/
 */
int intlog2(int x)
{
    int log = 0;
    if (x < 0) { x = 0; }
    while ((x >> log) > 0)
    {
        log++;
    }
    if (log > 0 && x == 1<<(log-1)) { log--; }
    return log;
}

void CH264StructReader::ReadPPS(CBitStreamReader& bs, pps_t& pps)
{
    memset(&pps,0,sizeof(pps_t));

    pps.pic_parameter_set_id = bs.ReadUE();
    pps.seq_parameter_set_id = bs.ReadUE();
    pps.entropy_coding_mode_flag = bs.ReadU1();
    pps.pic_order_present_flag = bs.ReadU1();
    pps.num_slice_groups_minus1 = bs.ReadUE();

    if (pps.num_slice_groups_minus1 > 0)
    {
        pps.slice_group_map_type = bs.ReadUE();
        if (pps.slice_group_map_type == 0)
        {
            for (int i_group = 0; i_group <= pps.num_slice_groups_minus1; i_group++)
                pps.run_length_minus1[i_group] = bs.ReadUE();
        }
        else if (pps.slice_group_map_type == 2)
        {
            for (int i_group = 0; i_group < pps.num_slice_groups_minus1; i_group++)
            {
                pps.top_left[i_group] = bs.ReadUE();
                pps.bottom_right[i_group] = bs.ReadUE();
            }
        }
        else if (pps.slice_group_map_type == 3 ||
                 pps.slice_group_map_type == 4 ||
                 pps.slice_group_map_type == 5)
        {
            pps.slice_group_change_direction_flag = bs.ReadU1();
            pps.slice_group_change_rate_minus1 = bs.ReadUE();
        }
        else if (pps.slice_group_map_type == 6)
        {
            pps.pic_size_in_map_units_minus1 = bs.ReadUE();
            for (int i = 0; i <= pps.pic_size_in_map_units_minus1; i++)
            {
                int v = intlog2(pps.num_slice_groups_minus1 + 1);
                pps.slice_group_id[i] = bs.ReadU(v);
            }
        }
    }
    pps.num_ref_idx_l0_active_minus1 = bs.ReadUE();
    pps.num_ref_idx_l1_active_minus1 = bs.ReadUE();
    pps.weighted_pred_flag = bs.ReadU1();
    pps.weighted_bipred_idc = bs.ReadU(2);
    pps.pic_init_qp_minus26 = bs.ReadSE();
    pps.pic_init_qs_minus26 = bs.ReadSE();
    pps.chroma_qp_index_offset = bs.ReadSE();
    pps.deblocking_filter_control_present_flag = bs.ReadU1();
    pps.constrained_intra_pred_flag = bs.ReadU1();
    pps.redundant_pic_cnt_present_flag = bs.ReadU1();

    pps.more_rbsp_data_present = MoreRbspData(bs);
    if (pps.more_rbsp_data_present)
    {
        pps.transform_8x8_mode_flag = bs.ReadU1();
        pps.pic_scaling_matrix_present_flag = bs.ReadU1();
        if (pps.pic_scaling_matrix_present_flag)
        {
            for (int i=0; i<6 + 2*pps.transform_8x8_mode_flag; i++)
            {
                pps.pic_scaling_list_present_flag[i] = bs.ReadU1();
                if (pps.pic_scaling_list_present_flag[i])
                {
                    if (i<6)
                        ReadScalingList(bs, pps.ScalingList4x4[i], 16, &(pps.UseDefaultScalingMatrix4x4Flag[i]));
                    else
                        ReadScalingList(bs, pps.ScalingList8x8[i-6], 64, &(pps.UseDefaultScalingMatrix8x8Flag[i-6]));
                }
            }
        }
        pps.second_chroma_qp_index_offset = bs.ReadSE();
    }
    
    ReadTrailingBits(bs);
}

REFERENCE_TIME CH264StructReader::GetAvgTimePerFrame(int num_units_in_tick, int time_scale)
{
    REFERENCE_TIME val = 0;
    // Trick for weird parameters
    // https://github.com/MediaPortal/MediaPortal-1/blob/master/DirectShowFilters/BDReader/source/FrameHeaderParser.cpp
    if ((num_units_in_tick < 1000) || (num_units_in_tick > 1001))
    {
        if  ((time_scale % num_units_in_tick != 0) && ((time_scale*1001) % num_units_in_tick == 0))
        {
            time_scale          = (time_scale * 1001) / num_units_in_tick;
            num_units_in_tick   = 1001;
        }
        else
        {
            time_scale          = (time_scale * 1000) / num_units_in_tick;
            num_units_in_tick   = 1000;
        }
    }

    // VUI consider fields even for progressive stream : multiply by 2!
    if (time_scale)
        val = 2 * (10000000I64*num_units_in_tick)/time_scale;

    return val;
}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation