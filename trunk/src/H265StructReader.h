//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : Konstantin Ivlev aka SSE4 (tomskside@gmail.com)
// 
//
//-----------------------------------------------------------------------------

#pragma once

GRAPHSTUDIO_NAMESPACE_START

#pragma pack(push, 1)
// ISO/IEC 14496-15:2013(E) 8.3.3.1.2 HEVCDecoderConfigurationRecord
typedef struct HEVCDecoderConfigurationRecord_t
{
	uint8 configurationVersion;
	uint8 general_profile_idc : 5;
	uint8 general_tier_flag : 1;
	uint8 general_profile_space : 2;
	uint8 general_profile_compatibility_flags[4];
	uint8 general_constraint_indicator_flags[6];
	uint8 general_level_idc;
	uint16 min_spatial_segmentation_idc : 12;
	uint16 reserved0 : 4;
	uint8 parallelismType : 2;
	uint8 reserved2 : 6;
	uint8 chromaFormat : 2;
	uint8 reserved3 : 6;
	uint8 bitDepthLumaMinus8 : 3;
	uint8 reserved4 : 5;
	uint8 bitDepthChromaMinus8 : 3;
	uint8 reserved5 : 5;
	uint16 avgFrameRate;
	uint8 lengthSizeMinusOne : 2;
	uint8 temporalIdNested : 1;
	uint8 numTemporalLayers : 3;
	uint8 constantFrameRate : 2;
	uint8 numOfArrays;
}
HEVCDecoderConfigurationRecord;
#pragma pack(pop)

// Table 7-1 – NAL unit type codes and NAL unit type classes
enum { kH265VPSNUT = 32 };
enum { kH265SPSNUT = 33 };
enum { kH265PPSNUT = 34 };

// 7.3.3 Profile, tier and level syntax
typedef struct h265ptl_t
{
	uint8 maxNumSubLayersMinus1;
	uint8 general_profile_space; // u(2)
	uint8 general_tier_flag; // u(1)
	uint8 general_profile_idc; // u(5)
	// for (j = 0; j < 32; j++)
		uint32 general_profile_compatibility_flag; // u(1)
	uint8 general_progressive_source_flag; // u(1)
	uint8 general_interlaced_source_flag; // u(1)
	uint8 general_non_packed_constraint_flag; // u(1)
	uint8 general_frame_only_constraint_flag; // u(1)
	// general_reserved_zero_44bits u(44) 
	uint8 general_level_idc; // u(8)
	// for (i = 0; i < maxNumSubLayersMinus1; i++) {
		uint8 sub_layer_profile_present_flag[8]; // u(1)
		uint8 sub_layer_level_present_flag[8]; // u(1)
	// if (maxNumSubLayersMinus1 > 0)
		// for (i = maxNumSubLayersMinus1; i < 8; i++)
			// reserved_zero_2bits[i] u(2)
	// for (i = 0; i < maxNumSubLayersMinus1; i++) {
		// if (sub_layer_profile_present_flag[i]) {
			uint8 sub_layer_profile_space[8]; // u(2)
			uint8 sub_layer_tier_flag[8]; // u(1)
			uint8 sub_layer_profile_idc[8]; // u(5)
			// for (j = 0; j < 32; j++)
				uint32 sub_layer_profile_compatibility_flag[8]; // u(1)
				uint8 sub_layer_progressive_source_flag[8]; // u(1)
				uint8 sub_layer_interlaced_source_flag[8]; // u(1)
				uint8 sub_layer_non_packed_constraint_flag[8]; // u(1)
				uint8 sub_layer_frame_only_constraint_flag[8]; // u(1)
	// sub_layer_reserved_zero_44bits[i] u(44)
	// if (sub_layer_level_present_flag[i])
		uint8 sub_layer_level_idc[8]; //  u(8)
}
h265ptl;

// E.2.2 HRD parameters syntax
typedef struct h265hrd_t
{
	// if (commonInfPresentFlag) {
		uint8 nal_hrd_parameters_present_flag; // u(1)
		uint8 vcl_hrd_parameters_present_flag; // u(1)
		// if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag){
			uint8 sub_pic_hrd_params_present_flag; // u(1)
			// if (sub_pic_hrd_params_present_flag) {
				uint8 tick_divisor_minus2; // u(8)
				uint8 du_cpb_removal_delay_increment_length_minus1; // u(5)
				uint8 sub_pic_cpb_params_in_pic_timing_sei_flag; // u(1)
				uint8 dpb_output_delay_du_length_minus1; // u(5)
			uint8 bit_rate_scale; // u(4)
			uint8 cpb_size_scale; // u(4)
			// if (sub_pic_hrd_params_present_flag)
				uint8 cpb_size_du_scale; // u(4)
			uint8 initial_cpb_removal_delay_length_minus1; // u(5)
			uint8 au_cpb_removal_delay_length_minus1; // u(5)
			uint8 dpb_output_delay_length_minus1; // u(5)
		//for (i = 0; i <= maxNumSubLayersMinus1; i++) {
			uint8 fixed_pic_rate_general_flag[8]; // u(1)
			// if (!fixed_pic_rate_general_flag[i])
				uint8 fixed_pic_rate_within_cvs_flag[8]; // u(1)
				// if (fixed_pic_rate_within_cvs_flag[i])
					uint32 elemental_duration_in_tc_minus1[8]; // ue(v)
				// else
					uint8 low_delay_hrd_flag[8]; // u(1)
				// if (!low_delay_hrd_flag[i])
					uint32 cpb_cnt_minus1[8]; // ue(v)
				//if (nal_hrd_parameters_present_flag)
					// sub_layer_hrd_parameters(i)
				// if (vcl_hrd_parameters_present_flag)
					// sub_layer_hrd_parameters(i)
}
h265hrd;

enum { kH265EXTENDEDSAR = 255 };

typedef struct h265vui_t
{
	h265hrd_t hrd;

	uint8 aspect_ratio_info_present_flag; // u(1)
	// if (aspect_ratio_info_present_flag) {
		uint8 aspect_ratio_idc; // u(8)
		// if (aspect_ratio_idc == EXTENDED_SAR) {
			uint16 sar_width; // u(16)
			uint16 sar_height; // u(16)
	uint8 overscan_info_present_flag; // u(1)
	// if (overscan_info_present_flag)
		uint8 overscan_appropriate_flag; // u(1)
	uint8 video_signal_type_present_flag; // u(1)
	// if (video_signal_type_present_flag) {
		uint8 video_format; // u(3)
		uint8 video_full_range_flag; // u(1)
		uint8 colour_description_present_flag; // u(1)
		// if (colour_description_present_flag) {
			uint8 colour_primaries; // u(8)
			uint8 transfer_characteristics; // u(8)
			uint8 matrix_coeffs; // u(8)
	uint8 chroma_loc_info_present_flag; // u(1)
	// if (chroma_loc_info_present_flag) {
		uint32 chroma_sample_loc_type_top_field; // ue(v)
		uint32 chroma_sample_loc_type_bottom_field; // ue(v)
	uint8 neutral_chroma_indication_flag; // u(1)
	uint8 field_seq_flag; // u(1)
	uint8 frame_field_info_present_flag; // u(1)
	uint8 default_display_window_flag; // u(1)
	// if (default_display_window_flag) {
		uint32 def_disp_win_left_offset; // ue(v)
		uint32 def_disp_win_right_offset; // ue(v)
		uint32 def_disp_win_top_offset; // ue(v)
		uint32 def_disp_win_bottom_offset; // ue(v)
	uint8 vui_timing_info_present_flag; // u(1)
	// if (vui_timing_info_present_flag) {
		uint32 vui_num_units_in_tick; // u(32)
		uint32 vui_time_scale; // u(32)
		uint8 vui_poc_proportional_to_timing_flag; // u(1)
		// if (vui_poc_proportional_to_timing_flag)
			uint32 vui_num_ticks_poc_diff_one_minus1; // ue(v)
			uint8 vui_hrd_parameters_present_flag; // u(1)
			// if (vui_hrd_parameters_present_flag)
				// hrd_parameters(1, sps_max_sub_layers_minus1)
	uint8 bitstream_restriction_flag; // u(1)
	// if (bitstream_restriction_flag) {
		uint8 tiles_fixed_structure_flag; // u(1)
		uint8 motion_vectors_over_pic_boundaries_flag; // u(1)
		uint8 restricted_ref_pic_lists_flag; // u(1)
		uint32 min_spatial_segmentation_idc; // ue(v)
		uint32 max_bytes_per_pic_denom; // ue(v)
		uint32 max_bits_per_min_cu_denom; // ue(v)
		uint32 log2_max_mv_length_horizontal; // ue(v)
		uint32 log2_max_mv_length_vertical; // ue(v)
}
h265vui;

enum { kH265MAXHRD = 1 };

// 7.3.2.1 Video parameter set RBSP syntax
typedef struct h265vps_t
{
	h265ptl_t ptl;
	// Although the value of vps_num_hrd_parameters is required to be less than or equal to 1 in this version of 
	// this Specification, decoders shall allow other values of vps_num_hrd_parameters in the range of 0 to 1024, inclusive, to appear in the syntax.
	h265hrd_t hrd[kH265MAXHRD];

	uint8 vps_video_parameter_set_id; // u(4)
	// vps_reserved_three_2bits u(2)
	uint8 vps_max_layers_minus1; // u(6)
	uint8 vps_max_sub_layers_minus1; // u(3)
	uint8 vps_temporal_id_nesting_flag; // u(1)
	// vps_reserved_0xffff_16bits u(16)
	uint8 vps_sub_layer_ordering_info_present_flag; // u(1)
	// for (i = (vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1); i <= vps_max_sub_layers_minus1; i++) {
		uint32 vps_max_dec_pic_buffering_minus1[8]; // ue(v)
		uint32 vps_max_num_reorder_pics[8]; // ue(v)
		uint32 vps_max_latency_increase_plus1[8]; // ue(v)
	uint8 vps_max_layer_id; // u(6)
	// Although the value of vps_num_layer_sets_minus1 is required to be equal to 0 in this version of this Specification, decoders shall
	// allow other values of vps_num_layer_sets_minus1 in the range of 0 to 1023, inclusive, to appear in the syntax.
	uint32 vps_num_layer_sets_minus1; // ue(v)
	// for (i = 1; i <= vps_num_layer_sets_minus1; i++)
		// for (j = 0; j <= vps_max_layer_id; j++)
			uint8 layer_id_included_flag[1024][64]; // u(1)
	uint8 vps_timing_info_present_flag; // u(1)
	// if (vps_timing_info_present_flag) {
		uint32 vps_num_units_in_tick; // u(32)
		uint32 vps_time_scale; // u(32)
		uint8 vps_poc_proportional_to_timing_flag; // u(1)
		// if (vps_poc_proportional_to_timing_flag)
			uint32 vps_num_ticks_poc_diff_one_minus1; // ue(v)
		uint32 vps_num_hrd_parameters; // ue(v)
		// for (i = 0; i < vps_num_hrd_parameters; i++) {
			// Although the value of vps_num_hrd_parameters is required to be less than or equal to 1 in this version of this Specification,
			// decoders shall allow other values of vps_num_hrd_parameters in the range of 0 to 1024, inclusive, to	appear in the syntax.
			uint32 hrd_layer_set_idx[kH265MAXHRD]; // ue(v)
			// if (i > 0)
				uint8 cprms_present_flag[kH265MAXHRD]; // u(1)
			// hrd_parameters(cprms_present_flag[i], vps_max_sub_layers_minus1)
	uint8 vps_extension_flag; // u(1)
	// if (vps_extension_flag)
		// while (more_rbsp_data())
			//vps_extension_data_flag u(1)
	// rbsp_trailing_bits()
}
h265vps;

// 7.3.2.2 Sequence parameter set RBSP syntax
typedef struct h265sps_t
{
	h265ptl_t ptl;
	h265vui_t vui;

	uint8 sps_video_parameter_set_id; // u(4)
	uint8 sps_max_sub_layers_minus1; // u(3)
	uint8 sps_temporal_id_nesting_flag; // u(1)
	uint32 sps_seq_parameter_set_id; // ue(v)
	uint32 chroma_format_idc; // ue(v)
	// if (chroma_format_idc = = 3)
		uint8 separate_colour_plane_flag; // u(1)
	uint32 pic_width_in_luma_samples; // ue(v)
	uint32 pic_height_in_luma_samples; // ue(v)
	uint8 conformance_window_flag; // u(1)
	// if (conformance_window_flag) {
		uint32 conf_win_left_offset; // ue(v)
		uint32 conf_win_right_offset; // ue(v)
		uint32 conf_win_top_offset; // ue(v)
		uint32 conf_win_bottom_offset; // ue(v)
	uint32 bit_depth_luma_minus8; // ue(v)
	uint32 bit_depth_chroma_minus8; // ue(v)
	uint32 log2_max_pic_order_cnt_lsb_minus4; // ue(v)
	uint8 sps_sub_layer_ordering_info_present_flag; // u(1)
	// for (i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1); i <= sps_max_sub_layers_minus1; i++) {
		uint32 sps_max_dec_pic_buffering_minus1[8]; // ue(v)
		uint32 sps_max_num_reorder_pics[8]; // ue(v)
		uint32 sps_max_latency_increase_plus1[8]; // ue(v)
	uint32 log2_min_luma_coding_block_size_minus3; // ue(v)
	uint32 log2_diff_max_min_luma_coding_block_size; // ue(v)
	uint32 log2_min_transform_block_size_minus2; // ue(v)
	uint32 log2_diff_max_min_transform_block_size; // ue(v)
	uint32 max_transform_hierarchy_depth_inter; // ue(v)
	uint32 max_transform_hierarchy_depth_intra; // ue(v)
	uint8 scaling_list_enabled_flag; // u(1)
	// if (scaling_list_enabled_flag) {
		uint8 sps_scaling_list_data_present_flag; // u(1)
		// if (sps_scaling_list_data_present_flag)
			// scaling_list_data()
	uint8 amp_enabled_flag; // u(1)
	uint8 sample_adaptive_offset_enabled_flag; // u(1)
	uint8 pcm_enabled_flag; // u(1)
	// if (pcm_enabled_flag) {
		uint8 pcm_sample_bit_depth_luma_minus1; // u(4)
		uint8 pcm_sample_bit_depth_chroma_minus1; // u(4)
		uint32 log2_min_pcm_luma_coding_block_size_minus3; // ue(v)
		uint32 log2_diff_max_min_pcm_luma_coding_block_size; // ue(v)
		uint8 pcm_loop_filter_disabled_flag; // u(1)
	// The value of num_short_term_ref_pic_sets shall be in the range of 0 to 64, inclusive.
	uint32 num_short_term_ref_pic_sets; // ue(v)
	// for (i = 0; i < num_short_term_ref_pic_sets; i++)
		// short_term_ref_pic_set[i]
	uint8 long_term_ref_pics_present_flag; // u(1)
	// if (long_term_ref_pics_present_flag) {
		// The value of num_long_term_ref_pics_sps shall be in the range of 0 to 32, inclusive.
		uint32 num_long_term_ref_pics_sps; // ue(v)
		// for (i = 0; i < num_long_term_ref_pics_sps; i++) {
			// The number of bits used to represent lt_ref_pic_poc_lsb_sps[ i ] is equal to log2_max_pic_order_cnt_lsb_minus4 + 4.
			uint32 lt_ref_pic_poc_lsb_sps[32]; // u(v)
			uint8 used_by_curr_pic_lt_sps_flag[32]; // u(1)
	uint8 sps_temporal_mvp_enabled_flag; // u(1)
	uint8 strong_intra_smoothing_enabled_flag; // u(1)
	uint8 vui_parameters_present_flag; // u(1)
	//if (vui_parameters_present_flag)
		//vui_parameters()
	uint8 sps_extension_flag; // u(1)
	// if (sps_extension_flag)
		// while (more_rbsp_data())
			// sps_extension_data_flag u(1)
	// rbsp_trailing_bits()
}
h265sps;

// 7.3.2.3 Picture parameter set RBSP syntax
typedef struct h265pps_t
{
	uint32 pps_pic_parameter_set_id; // ue(v)
	uint32 pps_seq_parameter_set_id; // ue(v)
	uint8 dependent_slice_segments_enabled_flag; // u(1)
	uint8 output_flag_present_flag; // u(1)
	uint8 num_extra_slice_header_bits; // u(3)
	uint8 sign_data_hiding_enabled_flag; // u(1)
	uint8 cabac_init_present_flag; // u(1)
	uint32 num_ref_idx_l0_default_active_minus1; // ue(v)
	uint32 num_ref_idx_l1_default_active_minus1; // ue(v)
	int32 init_qp_minus26; // se(v)
	uint8 constrained_intra_pred_flag; // u(1)
	uint8 transform_skip_enabled_flag; // u(1)
	uint8 cu_qp_delta_enabled_flag; // u(1)
	// if (cu_qp_delta_enabled_flag)
		uint32 diff_cu_qp_delta_depth; // ue(v)
	int32 pps_cb_qp_offset; // se(v)
	int32 pps_cr_qp_offset; // se(v)
	uint8 pps_slice_chroma_qp_offsets_present_flag; // u(1)
	uint8 weighted_pred_flag; // u(1)
	uint8 weighted_bipred_flag; // u(1)
	uint8 transquant_bypass_enabled_flag; // u(1)
	uint8 tiles_enabled_flag; // u(1)
	uint8 entropy_coding_sync_enabled_flag; // u(1)
	// if (tiles_enabled_flag) {
		uint32 num_tile_columns_minus1; // ue(v)
		uint32 num_tile_rows_minus1; // ue(v)
		uint8 uniform_spacing_flag; // u(1)
		// if (!uniform_spacing_flag) {
		//for (i = 0; i < num_tile_columns_minus1; i++)
			uint32 column_width_minus1[1024]; // ue(v)
		// for (i = 0; i < num_tile_rows_minus1; i++)
			uint32 row_height_minus1[1024]; // ue(v)
		uint8 loop_filter_across_tiles_enabled_flag; // u(1)	
	uint8 pps_loop_filter_across_slices_enabled_flag; // u(1)
	uint8 deblocking_filter_control_present_flag; // u(1)
	// if (deblocking_filter_control_present_flag) {
		uint8 deblocking_filter_override_enabled_flag; // u(1)
		uint8 pps_deblocking_filter_disabled_flag; // u(1)
		// if (!pps_deblocking_filter_disabled_flag) {
			int32 pps_beta_offset_div2; // se(v)
			int32 pps_tc_offset_div2; // se(v)
	uint8 pps_scaling_list_data_present_flag; // u(1)
	// if (pps_scaling_list_data_present_flag)
		//scaling_list_data()
	uint8 lists_modification_present_flag; // u(1)
	uint32 log2_parallel_merge_level_minus2; // ue(v)
	uint8 slice_segment_header_extension_present_flag; // u(1)
	uint8 pps_extension_flag; // u(1)
	//if (pps_extension_flag)
		// while (more_rbsp_data())
			// pps_extension_data_flag u(1)
	//rbsp_trailing_bits()
}
h265pps;

class CH265StructReader
{
public:
	static void ReadVPS(CBitStreamReader& bs, h265vps_t& vps);
	static void ReadSPS(CBitStreamReader& bs, h265sps_t& sps);
	static void ReadPPS(CBitStreamReader& bs, h265pps_t& pps);
	static void ReadVUI(CBitStreamReader& bs, h265vui_t& vui, uint8 maxNumSubLayersMinus1);
	static void ReadHRD(CBitStreamReader& bs, h265hrd_t& hrd, uint8 commonInfPresentFlag, uint8 maxNumSubLayersMinus1);
	static void ReadProfileTierLevel(CBitStreamReader& bs, h265ptl_t& ptl, uint8 maxNumSubLayersMinus1);
};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation
