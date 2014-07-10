//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : Konstantin Ivlev aka SSE4 (tomskside@gmail.com)
//  
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "H264StructReader.h"

#pragma warning(disable: 4800)	// disables the warning

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

void CH265StructReader::ReadVPS(CBitStreamReader& bs, h265vps_t& vps)
{
	memset(&vps, 0, sizeof(h265vps_t));
	vps.vps_video_parameter_set_id = bs.ReadU(4);
	bs.SkipU(2);
	vps.vps_max_layers_minus1 = bs.ReadU(6);
	vps.vps_max_sub_layers_minus1 = bs.ReadU(3);
	vps.vps_temporal_id_nesting_flag = bs.ReadU1();
	bs.SkipU8(2);
	ReadProfileTierLevel(bs, vps.ptl, vps.vps_max_sub_layers_minus1);
	vps.vps_sub_layer_ordering_info_present_flag = bs.ReadU1();
	for (uint8 i = (vps.vps_sub_layer_ordering_info_present_flag ? 0 : vps.vps_max_sub_layers_minus1); i < vps.vps_max_layers_minus1; ++i)
	{
		vps.vps_max_dec_pic_buffering_minus1[i] = bs.ReadUE();
		vps.vps_max_num_reorder_pics[i] = bs.ReadUE();
		vps.vps_max_latency_increase_plus1[i] = bs.ReadUE();
	}
	vps.vps_max_layer_id = bs.ReadU(6);
	vps.vps_num_layer_sets_minus1 = bs.ReadUE();
	vps.vps_num_layer_sets_minus1 = min(1023, vps.vps_num_layer_sets_minus1);
	for (uint32 i(1); i <= vps.vps_num_layer_sets_minus1; ++i)
	for (uint8 j(0); j < vps.vps_max_layer_id; ++j)
	{
		vps.layer_id_included_flag[i][j] = bs.ReadU1();
	}
	if (vps.vps_timing_info_present_flag = bs.ReadU1())
	{
		vps.vps_num_units_in_tick = bs.ReadU32();
		vps.vps_time_scale = bs.ReadU32();
		if (vps.vps_poc_proportional_to_timing_flag = bs.ReadU1())
		{
			vps.vps_num_ticks_poc_diff_one_minus1 = bs.ReadUE();
		}
		vps.vps_num_hrd_parameters = bs.ReadUE();
		vps.vps_num_hrd_parameters = min(kH265MAXHRD, vps.vps_num_hrd_parameters);
		for (uint32 i(0); i < vps.vps_num_hrd_parameters; ++i)
		{
			vps.hrd_layer_set_idx[i] = bs.ReadUE();
			vps.cprms_present_flag[i] = (i > 0) ? bs.ReadU1() : 0;
			ReadHRD(bs, vps.hrd[i], vps.cprms_present_flag[i], vps.vps_max_sub_layers_minus1);
		}
	}
	vps.vps_extension_flag = bs.ReadU1();
}

void CH265StructReader::ReadSPS(CBitStreamReader& bs, h265sps_t& sps)
{
	memset(&sps, 0, sizeof(h265sps_t));
	sps.sps_video_parameter_set_id = bs.ReadU(4);
	sps.sps_max_sub_layers_minus1 = bs.ReadU(3);
	sps.sps_temporal_id_nesting_flag = bs.ReadU1();
	ReadProfileTierLevel(bs, sps.ptl, sps.sps_max_sub_layers_minus1);
	sps.sps_seq_parameter_set_id = bs.ReadUE();
	sps.chroma_format_idc = bs.ReadUE();
	if (sps.chroma_format_idc == 3) sps.separate_colour_plane_flag = bs.ReadU1();
	sps.pic_width_in_luma_samples = bs.ReadUE();
	sps.pic_height_in_luma_samples = bs.ReadUE();
	if (sps.conformance_window_flag = bs.ReadU1())
	{
		sps.conf_win_left_offset = bs.ReadUE();
		sps.conf_win_right_offset = bs.ReadUE();
		sps.conf_win_top_offset = bs.ReadUE();
		sps.conf_win_bottom_offset = bs.ReadUE();
	}
	sps.bit_depth_luma_minus8 = bs.ReadUE();
	sps.bit_depth_chroma_minus8 = bs.ReadUE();
	sps.log2_max_pic_order_cnt_lsb_minus4 = bs.ReadUE();
	sps.sps_sub_layer_ordering_info_present_flag = bs.ReadU1();
	for (uint8 i = (sps.sps_sub_layer_ordering_info_present_flag ? 0 : sps.sps_max_sub_layers_minus1); i <= sps.sps_max_sub_layers_minus1; ++i)
	{
		sps.sps_max_dec_pic_buffering_minus1[i] = bs.ReadUE();
		sps.sps_max_num_reorder_pics[i] = bs.ReadUE();
		sps.sps_max_latency_increase_plus1[i] = bs.ReadUE();
	}
	sps.log2_min_luma_coding_block_size_minus3 = bs.ReadUE();
	sps.log2_diff_max_min_luma_coding_block_size = bs.ReadUE();
	sps.log2_min_transform_block_size_minus2 = bs.ReadUE();
	sps.log2_diff_max_min_transform_block_size = bs.ReadUE();
	sps.max_transform_hierarchy_depth_inter = bs.ReadUE();
	sps.max_transform_hierarchy_depth_intra = bs.ReadUE();
	if (sps.scaling_list_enabled_flag = bs.ReadU1())
	{
		sps.sps_scaling_list_data_present_flag = bs.ReadU1();
	}
	sps.amp_enabled_flag = bs.ReadU1();
	sps.sample_adaptive_offset_enabled_flag = bs.ReadU1();
	if (sps.pcm_enabled_flag = bs.ReadU1())
	{
		sps.pcm_sample_bit_depth_luma_minus1 = bs.ReadU(4);
		sps.pcm_sample_bit_depth_chroma_minus1 = bs.ReadU(4);
		sps.log2_min_pcm_luma_coding_block_size_minus3 = bs.ReadUE();
		sps.log2_diff_max_min_pcm_luma_coding_block_size = bs.ReadUE();
		sps.pcm_loop_filter_disabled_flag = bs.ReadU1();
	}
	sps.num_short_term_ref_pic_sets = bs.ReadUE();
	if (sps.long_term_ref_pics_present_flag = bs.ReadU1())
	{
		sps.num_long_term_ref_pics_sps = bs.ReadUE();
		sps.num_long_term_ref_pics_sps = min(32, sps.num_long_term_ref_pics_sps);
		for (uint32 i(0); i < sps.num_long_term_ref_pics_sps; ++i)
		{
			sps.lt_ref_pic_poc_lsb_sps[i] = bs.ReadU(4 + sps.log2_max_pic_order_cnt_lsb_minus4);
			sps.used_by_curr_pic_lt_sps_flag[i] = bs.ReadU1();
		}
	}
	sps.sps_temporal_mvp_enabled_flag = bs.ReadU1();
	sps.strong_intra_smoothing_enabled_flag = bs.ReadU1();
	if (sps.vui_parameters_present_flag = bs.ReadU1())
	{
		ReadVUI(bs, sps.vui, sps.sps_max_sub_layers_minus1);
	}
	sps.sps_extension_flag = bs.ReadU1();
}

void CH265StructReader::ReadPPS(CBitStreamReader& bs, h265pps_t& pps)
{
	memset(&pps, 0, sizeof(h265pps_t));
	pps.pps_pic_parameter_set_id = bs.ReadUE();
	pps.pps_seq_parameter_set_id = bs.ReadUE();
	pps.dependent_slice_segments_enabled_flag = bs.ReadU1();
	pps.output_flag_present_flag = bs.ReadU1();
	pps.num_extra_slice_header_bits = bs.ReadU(3);
	pps.sign_data_hiding_enabled_flag = bs.ReadU1();
	pps.cabac_init_present_flag = bs.ReadU1();
	pps.num_ref_idx_l0_default_active_minus1 = bs.ReadUE();
	pps.num_ref_idx_l1_default_active_minus1 = bs.ReadUE();
	pps.init_qp_minus26 = bs.ReadSE();
	pps.constrained_intra_pred_flag = bs.ReadU1();
	pps.transform_skip_enabled_flag = bs.ReadU1();
	if (pps.cu_qp_delta_enabled_flag = bs.ReadU1()) pps.diff_cu_qp_delta_depth = bs.ReadUE();
	pps.pps_cb_qp_offset = bs.ReadSE();
	pps.pps_cr_qp_offset = bs.ReadSE();
	pps.pps_slice_chroma_qp_offsets_present_flag = bs.ReadU1();
	pps.weighted_pred_flag = bs.ReadU1();
	pps.weighted_bipred_flag = bs.ReadU1();
	pps.transquant_bypass_enabled_flag = bs.ReadU1();
	pps.tiles_enabled_flag = bs.ReadU1();
	pps.entropy_coding_sync_enabled_flag = bs.ReadU1();
	if (pps.tiles_enabled_flag)
	{
		pps.num_tile_columns_minus1 = bs.ReadUE();
		pps.num_tile_columns_minus1 = min(pps.num_tile_columns_minus1, 1024);
		pps.num_tile_rows_minus1 = bs.ReadUE();
		pps.num_tile_rows_minus1 = min(pps.num_tile_rows_minus1, 1024);
		pps.uniform_spacing_flag = bs.ReadU1();
		if (!pps.uniform_spacing_flag)
		{
			for (uint32 i(0); i < pps.num_tile_columns_minus1; i++) pps.column_width_minus1[i] = bs.ReadUE();
			for (uint32 i(0); i < pps.num_tile_rows_minus1; i++) pps.row_height_minus1[i] = bs.ReadUE();
		}
		pps.loop_filter_across_tiles_enabled_flag = bs.ReadU1();
	}
	pps.pps_loop_filter_across_slices_enabled_flag = bs.ReadU1();
	if (pps.deblocking_filter_control_present_flag = bs.ReadU1())
	{
		pps.deblocking_filter_override_enabled_flag = bs.ReadU1();
		pps.pps_deblocking_filter_disabled_flag = bs.ReadU1();
		if (!pps.pps_deblocking_filter_disabled_flag)
		{
			pps.pps_beta_offset_div2 = bs.ReadSE();
			pps.pps_tc_offset_div2 = bs.ReadSE();
		}
	}
	pps.pps_scaling_list_data_present_flag = bs.ReadU1();
	pps.lists_modification_present_flag = bs.ReadU1();
	pps.log2_parallel_merge_level_minus2 = bs.ReadUE();
	pps.slice_segment_header_extension_present_flag = bs.ReadU1();
	pps.pps_extension_flag = bs.ReadU1();
}

void CH265StructReader::ReadVUI(CBitStreamReader& bs, h265vui_t& vui, uint8 maxNumSubLayersMinus1)
{
	if (vui.aspect_ratio_info_present_flag = bs.ReadU1())
	{
		vui.aspect_ratio_idc = bs.ReadU8();
		if (vui.aspect_ratio_idc == kH265EXTENDEDSAR)
		{
			vui.sar_width = bs.ReadU16();
			vui.sar_height = bs.ReadU16();
		}
	}
	if (vui.overscan_info_present_flag = bs.ReadU1()) vui.overscan_appropriate_flag = bs.ReadU1();
	if (vui.video_signal_type_present_flag = bs.ReadU1())
	{
		vui.video_format = bs.ReadU(3);
		vui.video_full_range_flag = bs.ReadU1();
		if (vui.colour_description_present_flag = bs.ReadU1())
		{
			vui.colour_primaries = bs.ReadU8();
			vui.transfer_characteristics = bs.ReadU8();
			vui.matrix_coeffs = bs.ReadU8();
		}
	}
	if (vui.chroma_loc_info_present_flag = bs.ReadU1())
	{
		vui.chroma_sample_loc_type_top_field = bs.ReadUE();
		vui.chroma_sample_loc_type_bottom_field = bs.ReadUE();
	}
	vui.neutral_chroma_indication_flag = bs.ReadU1();
	vui.field_seq_flag = bs.ReadU1();
	vui.frame_field_info_present_flag = bs.ReadU1();
	if (vui.default_display_window_flag = bs.ReadU1())
	{
		vui.def_disp_win_left_offset = bs.ReadUE();
		vui.def_disp_win_right_offset = bs.ReadUE();
		vui.def_disp_win_top_offset = bs.ReadUE();
		vui.def_disp_win_bottom_offset = bs.ReadUE();
	}
	if (vui.vui_timing_info_present_flag = bs.ReadU1())
	{
		vui.vui_num_units_in_tick = bs.ReadU32();
		vui.vui_time_scale = bs.ReadU32();
		if (vui.vui_poc_proportional_to_timing_flag = bs.ReadU1())
		{
			vui.vui_num_ticks_poc_diff_one_minus1 = bs.ReadUE();
			if (vui.vui_hrd_parameters_present_flag = bs.ReadU1()) ReadHRD(bs, vui.hrd, 1, maxNumSubLayersMinus1);
		}
		if (vui.bitstream_restriction_flag = bs.ReadU1())
		{
			vui.tiles_fixed_structure_flag = bs.ReadU1();
			vui.motion_vectors_over_pic_boundaries_flag = bs.ReadU1();
			vui.restricted_ref_pic_lists_flag = bs.ReadU1();
			vui.min_spatial_segmentation_idc = bs.ReadUE();
			vui.max_bytes_per_pic_denom = bs.ReadUE();
			vui.max_bits_per_min_cu_denom = bs.ReadUE();
			vui.log2_max_mv_length_horizontal = bs.ReadUE();
			vui.log2_max_mv_length_vertical = bs.ReadUE();
		}
	}
}

void CH265StructReader::ReadHRD(CBitStreamReader& bs, h265hrd_t& hrd, uint8 commonInfPresentFlag, uint8 maxNumSubLayersMinus1)
{
	if (commonInfPresentFlag)
	{
		hrd.nal_hrd_parameters_present_flag = bs.ReadU1();
		hrd.vcl_hrd_parameters_present_flag = bs.ReadU1();
		if (hrd.nal_hrd_parameters_present_flag || hrd.vcl_hrd_parameters_present_flag)
		{
			if (hrd.sub_pic_hrd_params_present_flag = bs.ReadU1())
			{
				hrd.tick_divisor_minus2 = bs.ReadU8();
				hrd.du_cpb_removal_delay_increment_length_minus1 = bs.ReadU(5);
				hrd.sub_pic_cpb_params_in_pic_timing_sei_flag = bs.ReadU1();
				hrd.dpb_output_delay_du_length_minus1 = bs.ReadU(5);
			}
			hrd.bit_rate_scale = bs.ReadU(4);
			hrd.cpb_size_scale = bs.ReadU(4);
			if (hrd.sub_pic_hrd_params_present_flag) hrd.cpb_size_du_scale = bs.ReadU(4);
			hrd.initial_cpb_removal_delay_length_minus1 = bs.ReadU(5);
			hrd.au_cpb_removal_delay_length_minus1 = bs.ReadU(5);
			hrd.dpb_output_delay_length_minus1 = bs.ReadU(5);
		}
	}
	for (uint8 i(0); i <= maxNumSubLayersMinus1; i++)
	{
		hrd.fixed_pic_rate_general_flag[i] = bs.ReadU1();
		if (!hrd.fixed_pic_rate_general_flag[i]) hrd.fixed_pic_rate_within_cvs_flag[i] = bs.ReadU1();
		if (hrd.fixed_pic_rate_within_cvs_flag[i]) hrd.elemental_duration_in_tc_minus1[i] = bs.ReadUE(); else hrd.low_delay_hrd_flag[i] = bs.ReadU1();
		if (!hrd.low_delay_hrd_flag[i]) hrd.cpb_cnt_minus1[i] = bs.ReadSE();
		if (hrd.nal_hrd_parameters_present_flag) { /* sub_layer_hrd_parameters(i) */ }
		if (hrd.vcl_hrd_parameters_present_flag) { /* sub_layer_hrd_parameters(i) */ }
	}
}

void CH265StructReader::ReadProfileTierLevel(CBitStreamReader& bs, h265ptl_t& ptl, uint8 maxNumSubLayersMinus1)
{
	ptl.maxNumSubLayersMinus1 = maxNumSubLayersMinus1;
	ptl.general_profile_space = bs.ReadU(2);
	ptl.general_tier_flag = bs.ReadU1();
	ptl.general_profile_idc = bs.ReadU(5);
	ptl.general_profile_compatibility_flag = bs.ReadU32();
	ptl.general_progressive_source_flag = bs.ReadU1();
	ptl.general_interlaced_source_flag = bs.ReadU1();
	ptl.general_non_packed_constraint_flag = bs.ReadU1();
	ptl.general_frame_only_constraint_flag = bs.ReadU1();
	bs.SkipU(44);
	ptl.general_level_idc = bs.ReadU8();
	for (uint8 i(0); i < maxNumSubLayersMinus1; ++i)
	{
		ptl.sub_layer_profile_present_flag[i] = bs.ReadU1();
		ptl.sub_layer_level_present_flag[i] = bs.ReadU1();
	}
	if (maxNumSubLayersMinus1) bs.SkipU(2 * (8 - maxNumSubLayersMinus1));
	for (uint8 i(0); i < maxNumSubLayersMinus1; ++i)
	{
		if (ptl.sub_layer_profile_present_flag[i])
		{
			ptl.sub_layer_profile_space[i] = bs.ReadU(2);
			ptl.sub_layer_tier_flag[i] = bs.ReadU1();
			ptl.sub_layer_profile_idc[i] = bs.ReadU(5);
			ptl.sub_layer_profile_compatibility_flag[i] = bs.ReadU32();
			ptl.sub_layer_progressive_source_flag[i] = bs.ReadU1();
			ptl.sub_layer_interlaced_source_flag[i] = bs.ReadU1();
			ptl.sub_layer_non_packed_constraint_flag[i] = bs.ReadU1();
			ptl.sub_layer_frame_only_constraint_flag[i] = bs.ReadU1();
			bs.SkipU(44);
		}
		if (ptl.sub_layer_level_present_flag[i])
		{
			ptl.sub_layer_level_idc[i] = bs.ReadU8();
		}
	}
}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation
