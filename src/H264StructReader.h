//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : cplussharp
//  Originally seen in http://sourceforge.net/projects/h264bitstream/
//
//-----------------------------------------------------------------------------

#pragma once

GRAPHSTUDIO_NAMESPACE_START

// HRD parameters (Appendix E.1.2)
typedef struct
{
    int cpb_cnt_minus1;
    int bit_rate_scale;
    int cpb_size_scale;
        int bit_rate_value_minus1[32]; // up to cpb_cnt_minus1, which is <= 31
        int cpb_size_value_minus1[32];
        bool cbr_flag[32];
    int initial_cpb_removal_delay_length_minus1;
    int cpb_removal_delay_length_minus1;
    int dpb_output_delay_length_minus1;
    int time_offset_length;
} hrd_t;

// VUI Parameters (Appendix E.1.1)
typedef struct
{
    bool aspect_ratio_info_present_flag;
        int aspect_ratio_idc;
        int sar_width;
        int sar_height;
    bool overscan_info_present_flag;
        bool overscan_appropriate_flag;
    bool video_signal_type_present_flag;
        int video_format;
        bool video_full_range_flag;
        bool colour_description_present_flag;
        int colour_primaries;
        int transfer_characteristics;
        int matrix_coefficients;
    bool chroma_loc_info_present_flag;
        int chroma_sample_loc_type_top_field;
        int chroma_sample_loc_type_bottom_field;
    bool timing_info_present_flag;
        int num_units_in_tick;
        int time_scale;
        bool fixed_frame_rate_flag;
    bool nal_hrd_parameters_present_flag;
        hrd_t nal_hrd_parameters;
    bool vcl_hrd_parameters_present_flag;
        hrd_t vcl_hrd_parameters;
        bool low_delay_hrd_flag;
    bool pic_struct_present_flag;
    bool bitstream_restriction_flag;
        bool motion_vectors_over_pic_boundaries_flag;
        int max_bytes_per_pic_denom;
        int max_bits_per_mb_denom;
        int log2_max_mv_length_horizontal;
        int log2_max_mv_length_vertical;
        int num_reorder_frames;
        int max_dec_frame_buffering;
} vui_t;

// Sequence Parameter Set (7.3.2.1)
typedef struct
{
    int profile_idc;
    bool constraint_set0_flag;
    bool constraint_set1_flag;
    bool constraint_set2_flag;
    bool constraint_set3_flag;
    bool constraint_set4_flag;
    bool constraint_set5_flag;
    int reserved_zero_2bits;
    int level_idc;
    int seq_parameter_set_id;
    int chroma_format_idc;
    int residual_colour_transform_flag;
    int bit_depth_luma_minus8;
    int bit_depth_chroma_minus8;
    int qpprime_y_zero_transform_bypass_flag;
    bool seq_scaling_matrix_present_flag;
      int seq_scaling_list_present_flag[8];
      int* ScalingList4x4[6];
      bool UseDefaultScalingMatrix4x4Flag[6];
      int* ScalingList8x8[2];
      bool UseDefaultScalingMatrix8x8Flag[2];
    int log2_max_frame_num_minus4;
    int pic_order_cnt_type;
      int log2_max_pic_order_cnt_lsb_minus4;
      bool delta_pic_order_always_zero_flag;
      int offset_for_non_ref_pic;
      int offset_for_top_to_bottom_field;
      int num_ref_frames_in_pic_order_cnt_cycle;
      int offset_for_ref_frame[256];
    int num_ref_frames;
    int gaps_in_frame_num_value_allowed_flag;
    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;
    bool frame_mbs_only_flag;
    bool mb_adaptive_frame_field_flag;
    bool direct_8x8_inference_flag;
    bool frame_cropping_flag;
      int frame_crop_left_offset;
      int frame_crop_right_offset;
      int frame_crop_top_offset;
      int frame_crop_bottom_offset;
    bool vui_parameters_present_flag;
    vui_t vui;
} sps_t;

// Picture Parameter Set (7.3.2.2)
typedef struct 
{
    int pic_parameter_set_id;
    int seq_parameter_set_id;
    bool entropy_coding_mode_flag;
    bool pic_order_present_flag;
    int num_slice_groups_minus1;
    int slice_group_map_type;
      int run_length_minus1[8]; // up to num_slice_groups_minus1, which is <= 7 in Baseline and Extended, 0 otheriwse
      int top_left[8];
      int bottom_right[8];
      bool slice_group_change_direction_flag;
      int slice_group_change_rate_minus1;
      int pic_size_in_map_units_minus1;
      int slice_group_id[256]; // FIXME what size?
    int num_ref_idx_l0_active_minus1;
    int num_ref_idx_l1_active_minus1;
    bool weighted_pred_flag;
    int weighted_bipred_idc;
    int pic_init_qp_minus26;
    int pic_init_qs_minus26;
    int chroma_qp_index_offset;
    bool deblocking_filter_control_present_flag;
    bool constrained_intra_pred_flag;
    bool redundant_pic_cnt_present_flag;

    bool more_rbsp_data_present;
    bool transform_8x8_mode_flag;
    bool pic_scaling_matrix_present_flag;
       bool pic_scaling_list_present_flag[8];
       int* ScalingList4x4[6];
       bool UseDefaultScalingMatrix4x4Flag[6];
       int* ScalingList8x8[2];
       bool UseDefaultScalingMatrix8x8Flag[2];
    int second_chroma_qp_index_offset;
} pps_t;


typedef struct
{
    int first_mb_in_slice;
    int slice_type;
    int pic_parameter_set_id;
    int frame_num;
    int field_pic_flag;
      int bottom_field_flag;
    int idr_pic_id;
    
    /*int pic_order_cnt_lsb;
    int delta_pic_order_cnt_bottom;
    int delta_pic_order_cnt[ 2 ];
    int redundant_pic_cnt;
    int direct_spatial_mv_pred_flag;
    int num_ref_idx_active_override_flag;
    int num_ref_idx_l0_active_minus1;
    int num_ref_idx_l1_active_minus1;
    int cabac_init_idc;
    int slice_qp_delta;
    int sp_for_switch_flag;
    int slice_qs_delta;
    int disable_deblocking_filter_idc;
    int slice_alpha_c0_offset_div2;
    int slice_beta_offset_div2;
    int slice_group_change_cycle;

    struct
    {
        int luma_log2_weight_denom;
        int chroma_log2_weight_denom;
        int luma_weight_l0_flag;
        int luma_weight_l0[64];
        int luma_offset_l0[64];
        int chroma_weight_l0_flag;
        int chroma_weight_l0[64][2];
        int chroma_offset_l0[64][2];
        int luma_weight_l1_flag;
        int luma_weight_l1[64];
        int luma_offset_l1[64];
        int chroma_weight_l1_flag;
        int chroma_weight_l1[64][2];
        int chroma_offset_l1[64][2];
    } pwt; // predictive weight table

    struct // FIXME stack or array
    {
        int ref_pic_list_reordering_flag_l0;
        int ref_pic_list_reordering_flag_l1;
        int reordering_of_pic_nums_idc;
        int abs_diff_pic_num_minus1;
        int long_term_pic_num;
    } rplr; // ref pic list reorder

    struct // FIXME stack or array
    {
        int no_output_of_prior_pics_flag;
        int long_term_reference_flag;
        int adaptive_ref_pic_marking_mode_flag;
        int memory_management_control_operation;
        int difference_of_pic_nums_minus1;
        int long_term_pic_num;
        int long_term_frame_idx;
        int max_long_term_frame_idx_plus1;
    } drpm; // decoded ref pic marking
    */
} slice_header_t;

typedef struct
{
    int payloadType;
    int payloadSize;
    BYTE* payload;
} sei_t;

enum SeiPayloadType 
{
    SEI_TYPE_BUFFERING_PERIOD   = 0,
    SEI_TYPE_PIC_TIMING         = 1,
    SEI_TYPE_PAN_SCAN_RECT      = 2,
    SEI_TYPE_FILLER_PAYLOAD     = 3,
    SEI_TYPE_USER_DATA_REGISTERED_ITU_T_T35 = 4,
    SEI_TYPE_USER_DATA_UNREGISTERED         = 5,
    SEI_TYPE_RECOVERY_POINT     = 6,
    SEI_TYPE_DEC_REF_PIC_MARKING_REPETITION = 7,
    SEI_TYPE_SPARE_PIC          = 8,
    SEI_TYPE_SCENE_INFO         = 9,
    SEI_TYPE_SUB_SEQ_INFO       = 10,
    SEI_TYPE_SUB_SEQ_LAYER_CHARACTERISTICS  = 11,
    SEI_TYPE_SUB_SEQ_CHARACTERISTICS        = 12,
    SEI_TYPE_FULL_FRAME_FREEZE  = 13,
    SEI_TYPE_FULL_FRAME_FREEZE_RELEASE      = 14,
    SEI_TYPE_FULL_FRAME_SNAPSHOT=15,
    SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_START   = 16,
    SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_END = 17,
    SEI_TYPE_MOTION_CONSTRAINED_SLICE_GROUP_SET = 18,
    SEI_TYPE_FILM_GRAIN_CHARACTERISTICS         = 19,
    SEI_TYPE_DEBLOCKING_FILTER_DISPLAY_PREFERENCE   = 20,
    SEI_TYPE_STEREO_VIDEO_INFO  = 21,
};

typedef struct
{
    bool clock_timestamp_flag;
        int ct_type;
        bool nuit_field_based_flag;
        int counting_type;
        bool full_timestamp_flag;
        bool discontinuity_flag;
        bool cnt_dropped_flag;
        int n_frames;

        int seconds_value;
        int minutes_value;
        int hours_value;

        bool seconds_flag;
        bool minutes_flag;
        bool hours_flag;

        int time_offset;
} picture_timestamp_t;

typedef struct
{
  int _is_initialized;
  int cpb_removal_delay;
  int dpb_output_delay;
  int pic_struct;
  picture_timestamp_t clock_timestamps[3]; // 3 is the maximum possible value
} sei_picture_timing_t;


class CH264StructReader
{
public:
    static void ReadTrailingBits(CBitStreamReader& bs);
    static bool MoreRbspData(CBitStreamReader& bs);
    static void ReadScalingList(CBitStreamReader& bs, int* scalingList, int sizeOfScalingList, bool* useDefaultScalingMatrixFlag);

    static void ReadHRD(CBitStreamReader& bs, hrd_t& hrd);
    static void ReadVUI(CBitStreamReader& bs, vui_t& vui);
    static void ReadSPS(CBitStreamReader& bs, sps_t& sps);
    static void ReadPPS(CBitStreamReader& bs, pps_t& pps);

    static void ReadSEI(CBitStreamReader& bs, sei_t& sei);
    static void ReadSliceHeader(CBitStreamReader& bs, slice_header_t& sh, sps_t& sps, bool isNonIDR);

    static REFERENCE_TIME GetAvgTimePerFrame(REFERENCE_TIME num_units_in_tick, REFERENCE_TIME time_scale);
	static RECT GetSize(sps_t& sps, bool ignoreCropping = false);
};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation