/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_filter.h"
#include "vp9/common/vp9_reconinter.h"
#include "vp9/common/vp9_reconintra.h"

void vp9_setup_scale_factors_for_frame(struct scale_factors *scale,
                                       YV12_BUFFER_CONFIG *other,
                                       int this_w, int this_h) {
  int other_h = other->y_crop_height;
  int other_w = other->y_crop_width;

  scale->x_num = other_w;
  scale->x_den = this_w;
  scale->x_offset_q4 = 0;  // calculated per-mb
  scale->x_step_q4 = 16 * other_w / this_w;

  scale->y_num = other_h;
  scale->y_den = this_h;
  scale->y_offset_q4 = 0;  // calculated per-mb
  scale->y_step_q4 = 16 * other_h / this_h;

  if (scale->x_num == scale->x_den && scale->y_num == scale->y_den) {
    scale->scale_value_x = unscaled_value;
    scale->scale_value_y = unscaled_value;
    scale->set_scaled_offsets = set_offsets_without_scaling;
    scale->scale_motion_vector_q3_to_q4 =
        motion_vector_q3_to_q4_without_scaling;
    scale->scale_motion_vector_component_q4 =
        motion_vector_component_q4_without_scaling;
  } else {
    scale->scale_value_x = scale_value_x_with_scaling;
    scale->scale_value_y = scale_value_y_with_scaling;
    scale->set_scaled_offsets = set_offsets_with_scaling;
    scale->scale_motion_vector_q3_to_q4 =
        motion_vector_q3_to_q4_with_scaling;
    scale->scale_motion_vector_component_q4 =
        motion_vector_component_q4_with_scaling;
  }

  // TODO(agrange): Investigate the best choice of functions to use here
  // for EIGHTTAP_SMOOTH. Since it is not interpolating, need to choose what
  // to do at full-pel offsets. The current selection, where the filter is
  // applied in one direction only, and not at all for 0,0, seems to give the
  // best quality, but it may be worth trying an additional mode that does
  // do the filtering on full-pel.
#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT
  if (scale->x_step_q4 == 16) {
    if (scale->y_step_q4 == 16) {
      // No scaling in either direction.
      scale->predict[0][0][0] = vp9_convolve_copy;
      scale->predict[0][0][1] = vp9_convolve_1by8;
      scale->predict[0][0][2] = vp9_convolve_qtr;
      scale->predict[0][0][3] = vp9_convolve_3by8;
      scale->predict[0][0][4] = vp9_convolve_avg;
      scale->predict[0][0][5] = vp9_convolve_5by8;
      scale->predict[0][0][6] = vp9_convolve_3qtr;
      scale->predict[0][0][7] = vp9_convolve_7by8;
      scale->predict[0][1][0] = vp9_convolve8_vert;
      scale->predict[0][1][1] = vp9_convolve8_1by8_vert;
      scale->predict[0][1][2] = vp9_convolve8_qtr_vert;
      scale->predict[0][1][3] = vp9_convolve8_3by8_vert;
      scale->predict[0][1][4] = vp9_convolve8_avg_vert;
      scale->predict[0][1][5] = vp9_convolve8_5by8_vert;
      scale->predict[0][1][6] = vp9_convolve8_3qtr_vert;
      scale->predict[0][1][7] = vp9_convolve8_7by8_vert;
      scale->predict[1][0][0] = vp9_convolve8_horiz;
      scale->predict[1][0][1] = vp9_convolve8_1by8_horiz;
      scale->predict[1][0][2] = vp9_convolve8_qtr_horiz;
      scale->predict[1][0][3] = vp9_convolve8_3by8_horiz;
      scale->predict[1][0][4] = vp9_convolve8_avg_horiz;
      scale->predict[1][0][5] = vp9_convolve8_5by8_horiz;
      scale->predict[1][0][6] = vp9_convolve8_3qtr_horiz;
      scale->predict[1][0][7] = vp9_convolve8_7by8_horiz;
    } else {
      // No scaling in x direction. Must always scale in the y direction.
      scale->predict[0][0][0] = vp9_convolve8_vert;
      scale->predict[0][0][1] = vp9_convolve8_1by8_vert;
      scale->predict[0][0][2] = vp9_convolve8_qtr_vert;
      scale->predict[0][0][3] = vp9_convolve8_3by8_vert;
      scale->predict[0][0][4] = vp9_convolve8_avg_vert;
      scale->predict[0][0][5] = vp9_convolve8_5by8_vert;
      scale->predict[0][0][6] = vp9_convolve8_3qtr_vert;
      scale->predict[0][0][7] = vp9_convolve8_7by8_vert;
      scale->predict[0][1][0] = vp9_convolve8_vert;
      scale->predict[0][1][1] = vp9_convolve8_1by8_vert;
      scale->predict[0][1][2] = vp9_convolve8_qtr_vert;
      scale->predict[0][1][3] = vp9_convolve8_3by8_vert;
      scale->predict[0][1][4] = vp9_convolve8_avg_vert;
      scale->predict[0][1][5] = vp9_convolve8_5by8_vert;
      scale->predict[0][1][6] = vp9_convolve8_3qtr_vert;
      scale->predict[0][1][7] = vp9_convolve8_7by8_vert;
      scale->predict[1][0][0] = vp9_convolve8;
      scale->predict[1][0][1] = vp9_convolve8_1by8;
      scale->predict[1][0][2] = vp9_convolve8_qtr;
      scale->predict[1][0][3] = vp9_convolve8_3by8;
      scale->predict[1][0][4] = vp9_convolve8_avg;
      scale->predict[1][0][5] = vp9_convolve8_5by8;
      scale->predict[1][0][6] = vp9_convolve8_3qtr;
      scale->predict[1][0][7] = vp9_convolve8_7by8;
    }
  } else {
    if (scale->y_step_q4 == 16) {
      // No scaling in the y direction. Must always scale in the x direction.
      scale->predict[0][0][0] = vp9_convolve8_horiz;
      scale->predict[0][0][1] = vp9_convolve8_1by8_horiz;
      scale->predict[0][0][2] = vp9_convolve8_qtr_horiz;
      scale->predict[0][0][3] = vp9_convolve8_3by8_horiz;
      scale->predict[0][0][4] = vp9_convolve8_avg_horiz;
      scale->predict[0][0][5] = vp9_convolve8_5by8_horiz;
      scale->predict[0][0][6] = vp9_convolve8_3qtr_horiz;
      scale->predict[0][0][7] = vp9_convolve8_7by8_horiz;
      scale->predict[0][1][0] = vp9_convolve8;
      scale->predict[0][1][1] = vp9_convolve8_1by8;
      scale->predict[0][1][2] = vp9_convolve8_qtr;
      scale->predict[0][1][3] = vp9_convolve8_3by8;
      scale->predict[0][1][4] = vp9_convolve8_avg;
      scale->predict[0][1][5] = vp9_convolve8_5by8;
      scale->predict[0][1][6] = vp9_convolve8_3qtr;
      scale->predict[0][1][7] = vp9_convolve8_7by8;
      scale->predict[1][0][0] = vp9_convolve8_horiz;
      scale->predict[1][0][1] = vp9_convolve8_1by8_horiz;
      scale->predict[1][0][2] = vp9_convolve8_qtr_horiz;
      scale->predict[1][0][3] = vp9_convolve8_3by8_horiz;
      scale->predict[1][0][4] = vp9_convolve8_avg_horiz;
      scale->predict[1][0][5] = vp9_convolve8_5by8_horiz;
      scale->predict[1][0][6] = vp9_convolve8_3qtr_horiz;
      scale->predict[1][0][7] = vp9_convolve8_7by8_horiz;
    } else {
      // Must always scale in both directions.
      scale->predict[0][0][0] = vp9_convolve8;
      scale->predict[0][0][1] = vp9_convolve8_1by8;
      scale->predict[0][0][2] = vp9_convolve8_qtr;
      scale->predict[0][0][3] = vp9_convolve8_3by8;
      scale->predict[0][0][4] = vp9_convolve8_avg;
      scale->predict[0][0][5] = vp9_convolve8_5by8;
      scale->predict[0][0][6] = vp9_convolve8_3qtr;
      scale->predict[0][0][7] = vp9_convolve8_7by8;
      scale->predict[0][1][0] = vp9_convolve8;
      scale->predict[0][1][1] = vp9_convolve8_1by8;
      scale->predict[0][1][2] = vp9_convolve8_qtr;
      scale->predict[0][1][3] = vp9_convolve8_3by8;
      scale->predict[0][1][4] = vp9_convolve8_avg;
      scale->predict[0][1][5] = vp9_convolve8_5by8;
      scale->predict[0][1][6] = vp9_convolve8_3qtr;
      scale->predict[0][1][7] = vp9_convolve8_7by8;
      scale->predict[1][0][0] = vp9_convolve8;
      scale->predict[1][0][1] = vp9_convolve8_1by8;
      scale->predict[1][0][2] = vp9_convolve8_qtr;
      scale->predict[1][0][3] = vp9_convolve8_3by8;
      scale->predict[1][0][4] = vp9_convolve8_avg;
      scale->predict[1][0][5] = vp9_convolve8_5by8;
      scale->predict[1][0][6] = vp9_convolve8_3qtr;
      scale->predict[1][0][7] = vp9_convolve8_7by8;
    }
  }
  // 2D subpel motion always gets filtered in both directions
  scale->predict[1][1][0] = vp9_convolve8;
  scale->predict[1][1][1] = vp9_convolve8_1by8;
  scale->predict[1][1][2] = vp9_convolve8_qtr;
  scale->predict[1][1][3] = vp9_convolve8_3by8;
  scale->predict[1][1][4] = vp9_convolve8_avg;
  scale->predict[1][1][5] = vp9_convolve8_5by8;
  scale->predict[1][1][6] = vp9_convolve8_3qtr;
  scale->predict[1][1][7] = vp9_convolve8_7by8;
}
#else
  if (scale->x_step_q4 == 16) {
    if (scale->y_step_q4 == 16) {
      // No scaling in either direction.
      scale->predict[0][0][0] = vp9_convolve_copy;
      scale->predict[0][0][1] = vp9_convolve_avg;
      scale->predict[0][1][0] = vp9_convolve8_vert;
      scale->predict[0][1][1] = vp9_convolve8_avg_vert;
      scale->predict[1][0][0] = vp9_convolve8_horiz;
      scale->predict[1][0][1] = vp9_convolve8_avg_horiz;
    } else {
      // No scaling in x direction. Must always scale in the y direction.
      scale->predict[0][0][0] = vp9_convolve8_vert;
      scale->predict[0][0][1] = vp9_convolve8_avg_vert;
      scale->predict[0][1][0] = vp9_convolve8_vert;
      scale->predict[0][1][1] = vp9_convolve8_avg_vert;
      scale->predict[1][0][0] = vp9_convolve8;
      scale->predict[1][0][1] = vp9_convolve8_avg;
    }
  } else {
    if (scale->y_step_q4 == 16) {
      // No scaling in the y direction. Must always scale in the x direction.
      scale->predict[0][0][0] = vp9_convolve8_horiz;
      scale->predict[0][0][1] = vp9_convolve8_avg_horiz;
      scale->predict[0][1][0] = vp9_convolve8;
      scale->predict[0][1][1] = vp9_convolve8_avg;
      scale->predict[1][0][0] = vp9_convolve8_horiz;
      scale->predict[1][0][1] = vp9_convolve8_avg_horiz;
    } else {
      // Must always scale in both directions.
      scale->predict[0][0][0] = vp9_convolve8;
      scale->predict[0][0][1] = vp9_convolve8_avg;
      scale->predict[0][1][0] = vp9_convolve8;
      scale->predict[0][1][1] = vp9_convolve8_avg;
      scale->predict[1][0][0] = vp9_convolve8;
      scale->predict[1][0][1] = vp9_convolve8_avg;
    }
  }
  // 2D subpel motion always gets filtered in both directions
  scale->predict[1][1][0] = vp9_convolve8;
  scale->predict[1][1][1] = vp9_convolve8_avg;
}
#endif

void vp9_setup_interp_filters(MACROBLOCKD *xd,
                              INTERPOLATIONFILTERTYPE mcomp_filter_type,
                              VP9_COMMON *cm) {
  if (xd->mode_info_context) {
    MB_MODE_INFO *mbmi = &xd->mode_info_context->mbmi;

    set_scale_factors(xd,
                      mbmi->ref_frame - 1,
                      mbmi->second_ref_frame - 1,
                      cm->active_ref_scale);
  }

  switch (mcomp_filter_type) {
    case EIGHTTAP:
    case SWITCHABLE:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8;
      break;
    case EIGHTTAP_SMOOTH:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8lp;
      break;
    case EIGHTTAP_SHARP:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8s;
      break;
    case BILINEAR:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_bilinear_filters;
      break;
#if CONFIG_ENABLE_6TAP
    case SIXTAP:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_6;
      break;
#endif
  }
  assert(((intptr_t)xd->subpix.filter_x & 0xff) == 0);
}

void vp9_copy_mem16x16_c(const uint8_t *src,
                         int src_stride,
                         uint8_t *dst,
                         int dst_stride) {
  int r;

  for (r = 0; r < 16; r++) {
#if !(CONFIG_FAST_UNALIGNED)
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
    dst[8] = src[8];
    dst[9] = src[9];
    dst[10] = src[10];
    dst[11] = src[11];
    dst[12] = src[12];
    dst[13] = src[13];
    dst[14] = src[14];
    dst[15] = src[15];

#else
    ((uint32_t *)dst)[0] = ((const uint32_t *)src)[0];
    ((uint32_t *)dst)[1] = ((const uint32_t *)src)[1];
    ((uint32_t *)dst)[2] = ((const uint32_t *)src)[2];
    ((uint32_t *)dst)[3] = ((const uint32_t *)src)[3];

#endif
    src += src_stride;
    dst += dst_stride;
  }
}

void vp9_copy_mem8x8_c(const uint8_t *src,
                       int src_stride,
                       uint8_t *dst,
                       int dst_stride) {
  int r;

  for (r = 0; r < 8; r++) {
#if !(CONFIG_FAST_UNALIGNED)
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
#else
    ((uint32_t *)dst)[0] = ((const uint32_t *)src)[0];
    ((uint32_t *)dst)[1] = ((const uint32_t *)src)[1];
#endif
    src += src_stride;
    dst += dst_stride;
  }
}

void vp9_copy_mem8x4_c(const uint8_t *src,
                       int src_stride,
                       uint8_t *dst,
                       int dst_stride) {
  int r;

  for (r = 0; r < 4; r++) {
#if !(CONFIG_FAST_UNALIGNED)
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
#else
    ((uint32_t *)dst)[0] = ((const uint32_t *)src)[0];
    ((uint32_t *)dst)[1] = ((const uint32_t *)src)[1];
#endif
    src += src_stride;
    dst += dst_stride;
  }
}

void vp9_build_inter_predictor(const uint8_t *src, int src_stride,
                               uint8_t *dst, int dst_stride,
                               const int_mv *mv_q3,
                               const struct scale_factors *scale,
                               int w, int h, int weight,
                               const struct subpix_fn_table *subpix) {
  int_mv32 mv = scale->scale_motion_vector_q3_to_q4(mv_q3, scale);
  src += (mv.as_mv.row >> 4) * src_stride + (mv.as_mv.col >> 4);
  scale->predict[!!(mv.as_mv.col & 15)][!!(mv.as_mv.row & 15)][weight](
      src, src_stride, dst, dst_stride,
      subpix->filter_x[mv.as_mv.col & 15], scale->x_step_q4,
      subpix->filter_y[mv.as_mv.row & 15], scale->y_step_q4,
      w, h);
}

/* Like vp9_build_inter_predictor, but takes the full-pel part of the
 * mv separately, and the fractional part as a q4.
 */
void vp9_build_inter_predictor_q4(const uint8_t *src, int src_stride,
                                  uint8_t *dst, int dst_stride,
                                  const int_mv *mv_q4,
                                  const struct scale_factors *scale,
                                  int w, int h, int weight,
                                  const struct subpix_fn_table *subpix) {
  const int scaled_mv_row_q4 =
      scale->scale_motion_vector_component_q4(mv_q4->as_mv.row,
                                              scale->y_num, scale->y_den,
                                              scale->y_offset_q4);
  const int scaled_mv_col_q4 =
      scale->scale_motion_vector_component_q4(mv_q4->as_mv.col,
                                              scale->x_num, scale->x_den,
                                              scale->x_offset_q4);
  const int subpel_x = scaled_mv_col_q4 & 15;
  const int subpel_y = scaled_mv_row_q4 & 15;

  src += (scaled_mv_row_q4 >> 4) * src_stride + (scaled_mv_col_q4 >> 4);
  scale->predict[!!subpel_x][!!subpel_y][weight](
      src, src_stride, dst, dst_stride,
      subpix->filter_x[subpel_x], scale->x_step_q4,
      subpix->filter_y[subpel_y], scale->y_step_q4,
      w, h);
}

static void build_2x1_inter_predictor_wh(const BLOCKD *d0, const BLOCKD *d1,
                                         struct scale_factors *s,
                                         uint8_t *predictor,
                                         int block_size, int stride,
                                         int which_mv, int weight,
                                         int width, int height,
                                         const struct subpix_fn_table *subpix,
                                         int row, int col) {
  struct scale_factors * scale = &s[which_mv];

  assert(d1->dst - d0->dst == block_size);
  assert(d1->pre == d0->pre + block_size);

  scale->set_scaled_offsets(scale, row, col);

  if (d0->bmi.as_mv[which_mv].as_int == d1->bmi.as_mv[which_mv].as_int) {
    uint8_t **base_pre = which_mv ? d0->base_second_pre : d0->base_pre;

    vp9_build_inter_predictor(*base_pre + d0->pre,
                              d0->pre_stride,
                              predictor, stride,
                              &d0->bmi.as_mv[which_mv],
                              scale,
                              width, height,
                              weight, subpix);

  } else {
    uint8_t **base_pre0 = which_mv ? d0->base_second_pre : d0->base_pre;
    uint8_t **base_pre1 = which_mv ? d1->base_second_pre : d1->base_pre;

    vp9_build_inter_predictor(*base_pre0 + d0->pre,
                              d0->pre_stride,
                              predictor, stride,
                              &d0->bmi.as_mv[which_mv],
                              scale,
                              width > block_size ? block_size : width, height,
                              weight, subpix);

    if (width <= block_size) return;

    scale->set_scaled_offsets(scale, row, col + block_size);

    vp9_build_inter_predictor(*base_pre1 + d1->pre,
                              d1->pre_stride,
                              predictor + block_size, stride,
                              &d1->bmi.as_mv[which_mv],
                              scale,
                              width - block_size, height,
                              weight, subpix);
  }
}

static void build_2x1_inter_predictor(const BLOCKD *d0, const BLOCKD *d1,
                                      struct scale_factors *s,
                                      int block_size, int stride,
                                      int which_mv, int weight,
                                      const struct subpix_fn_table *subpix,
                                      int row, int col) {
  uint8_t *d0_predictor = *(d0->base_dst) + d0->dst;
  uint8_t *d1_predictor = *(d1->base_dst) + d1->dst;
  struct scale_factors * scale = &s[which_mv];
  stride = d0->dst_stride;

  assert(d1_predictor - d0_predictor == block_size);
  assert(d1->pre == d0->pre + block_size);

  scale->set_scaled_offsets(scale, row, col);

  if (d0->bmi.as_mv[which_mv].as_int == d1->bmi.as_mv[which_mv].as_int) {
    uint8_t **base_pre = which_mv ? d0->base_second_pre : d0->base_pre;

    vp9_build_inter_predictor(*base_pre + d0->pre,
                              d0->pre_stride,
                              d0_predictor, stride,
                              &d0->bmi.as_mv[which_mv],
                              scale,
                              2 * block_size, block_size,
                              weight, subpix);
  } else {
    uint8_t **base_pre0 = which_mv ? d0->base_second_pre : d0->base_pre;
    uint8_t **base_pre1 = which_mv ? d1->base_second_pre : d1->base_pre;

    vp9_build_inter_predictor(*base_pre0 + d0->pre,
                              d0->pre_stride,
                              d0_predictor, stride,
                              &d0->bmi.as_mv[which_mv],
                              scale,
                              block_size, block_size,
                              weight, subpix);

    scale->set_scaled_offsets(scale, row, col + block_size);

    vp9_build_inter_predictor(*base_pre1 + d1->pre,
                              d1->pre_stride,
                              d1_predictor, stride,
                              &d1->bmi.as_mv[which_mv],
                              scale,
                              block_size, block_size,
                              weight, subpix);
  }
}

static void clamp_mv_to_umv_border(MV *mv, const MACROBLOCKD *xd) {
  /* If the MV points so far into the UMV border that no visible pixels
   * are used for reconstruction, the subpel part of the MV can be
   * discarded and the MV limited to 16 pixels with equivalent results.
   *
   * This limit kicks in at 19 pixels for the top and left edges, for
   * the 16 pixels plus 3 taps right of the central pixel when subpel
   * filtering. The bottom and right edges use 16 pixels plus 2 pixels
   * left of the central pixel when filtering.
   */
  if (mv->col < (xd->mb_to_left_edge - ((16 + VP9_INTERP_EXTEND) << 3)))
    mv->col = xd->mb_to_left_edge - (16 << 3);
  else if (mv->col > xd->mb_to_right_edge + ((15 + VP9_INTERP_EXTEND) << 3))
    mv->col = xd->mb_to_right_edge + (16 << 3);

  if (mv->row < (xd->mb_to_top_edge - ((16 + VP9_INTERP_EXTEND) << 3)))
    mv->row = xd->mb_to_top_edge - (16 << 3);
  else if (mv->row > xd->mb_to_bottom_edge + ((15 + VP9_INTERP_EXTEND) << 3))
    mv->row = xd->mb_to_bottom_edge + (16 << 3);
}

/* A version of the above function for chroma block MVs.*/
static void clamp_uvmv_to_umv_border(MV *mv, const MACROBLOCKD *xd) {
  const int extend = VP9_INTERP_EXTEND;

  mv->col = (2 * mv->col < (xd->mb_to_left_edge - ((16 + extend) << 3))) ?
            (xd->mb_to_left_edge - (16 << 3)) >> 1 : mv->col;
  mv->col = (2 * mv->col > xd->mb_to_right_edge + ((15 + extend) << 3)) ?
            (xd->mb_to_right_edge + (16 << 3)) >> 1 : mv->col;

  mv->row = (2 * mv->row < (xd->mb_to_top_edge - ((16 + extend) << 3))) ?
            (xd->mb_to_top_edge - (16 << 3)) >> 1 : mv->row;
  mv->row = (2 * mv->row > xd->mb_to_bottom_edge + ((15 + extend) << 3)) ?
            (xd->mb_to_bottom_edge + (16 << 3)) >> 1 : mv->row;
}

#if !CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT
// TODO(jkoleszar): yet another mv clamping function :-(
MV clamp_mv_to_umv_border_sb(const MV *src_mv,
    int bwl, int bhl,
    int mb_to_left_edge, int mb_to_top_edge,
    int mb_to_right_edge, int mb_to_bottom_edge) {
  /* If the MV points so far into the UMV border that no visible pixels
   * are used for reconstruction, the subpel part of the MV can be
   * discarded and the MV limited to 16 pixels with equivalent results.
   */
  const int epel_left = (VP9_INTERP_EXTEND + (4 << bwl)) << 3;
  const int epel_right = epel_left - (1 << 3);
  const int epel_top = (VP9_INTERP_EXTEND + (4 << bhl)) << 3;
  const int epel_bottom = epel_top - (1 << 3);
  MV clamped_mv;
  clamped_mv.col = clamp(src_mv->col,
                         mb_to_left_edge - epel_left,
                         mb_to_right_edge + epel_right);
  clamped_mv.row = clamp(src_mv->row,
                         mb_to_top_edge - epel_top,
                         mb_to_bottom_edge + epel_bottom);
  return clamped_mv;
}

struct build_inter_predictors_args {
  MACROBLOCKD *xd;
  uint8_t* dst[MAX_MB_PLANE];
  int dst_stride[MAX_MB_PLANE];
  int x;
  int y;
};
static void build_inter_predictors(int plane, int block,
                                   BLOCK_SIZE_TYPE bsize,
                                   int pred_w, int pred_h,
                                   void *argv) {
  const struct build_inter_predictors_args* const arg = argv;
  const int bwl = pred_w, bw = 4 << bwl;
  const int bhl = pred_h, bh = 4 << bhl;
  const int x_idx = block & ((1 << bwl) - 1), y_idx = block >> bwl;
  const int x = x_idx * 4, y = y_idx * 4;
  MACROBLOCKD * const xd = arg->xd;
  const int use_second_ref = xd->mode_info_context->mbmi.second_ref_frame > 0;
  int which_mv;

  for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
    const MV* const mv = (xd->mode_info_context->mbmi.mode == SPLITMV)
         ? &xd->block[block].bmi.as_mv[which_mv].as_mv
         : &xd->mode_info_context->mbmi.mv[which_mv].as_mv;

    const uint8_t * const base_pre = which_mv ? xd->second_pre.y_buffer
                                             : xd->pre.y_buffer;
    const int pre_stride = which_mv ? xd->second_pre.y_stride
                                    : xd->pre.y_stride;
    const uint8_t *const pre = base_pre +
        scaled_buffer_offset(x, y, pre_stride, &xd->scale_factor[which_mv]);
    struct scale_factors * const scale =
      plane == 0 ? &xd->scale_factor[which_mv] : &xd->scale_factor_uv[which_mv];

    int_mv clamped_mv;
    clamped_mv.as_mv = clamp_mv_to_umv_border_sb(mv, bwl, bhl,
                                                 xd->mb_to_left_edge,
                                                 xd->mb_to_top_edge,
                                                 xd->mb_to_right_edge,
                                                 xd->mb_to_bottom_edge);

    scale->set_scaled_offsets(scale, arg->y + y, arg->x + x);

    vp9_build_inter_predictor(pre, pre_stride,
                              arg->dst[plane], arg->dst_stride[plane],
                              &clamped_mv, &xd->scale_factor[which_mv],
                              bw, bh, which_mv, &xd->subpix);
  }
}
void vp9_build_inter_predictors_sby(MACROBLOCKD *xd,
                                    uint8_t *dst_y,
                                    int dst_ystride,
                                    int mb_row,
                                    int mb_col,
                                    BLOCK_SIZE_TYPE bsize) {
  struct build_inter_predictors_args args = {
    xd, {dst_y, NULL, NULL}, {dst_ystride, 0, 0}, mb_col * 16, mb_row * 16
  };
  foreach_predicted_block_in_plane(xd, bsize, 0, build_inter_predictors, &args);
}
#endif

#define AVERAGE_WEIGHT  (1 << (2 * CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT))

#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT

// Whether to use implicit weighting for UV
#define USE_IMPLICIT_WEIGHT_UV

// Whether to use implicit weighting for SplitMV
// #define USE_IMPLICIT_WEIGHT_SPLITMV

// #define SEARCH_MIN3
static int64_t get_consistency_metric(MACROBLOCKD *xd,
                                      uint8_t *tmp_y, int tmp_ystride) {
  int block_size = 16 <<  xd->mode_info_context->mbmi.sb_type;
  uint8_t *rec_y = xd->dst.y_buffer;
  int rec_ystride = xd->dst.y_stride;
  int64_t metric = 0;
  int i;
  if (xd->up_available) {
    for (i = 0; i < block_size; ++i) {
      int diff = abs(*(rec_y - rec_ystride + i) -
                     *(tmp_y + i));
#ifdef SEARCH_MIN3
      // Searches for the min abs diff among 3 pixel neighbors in the border
      int diff1 = xd->left_available ?
          abs(*(rec_y - rec_ystride + i - 1) - *(tmp_y + i)) : diff;
      int diff2 = i < block_size - 1 ?
          abs(*(rec_y - rec_ystride + i + 1) - *(tmp_y + i)) : diff;
      diff = diff <= diff1 ? diff : diff1;
      diff = diff <= diff2 ? diff : diff2;
#endif
      metric += diff;
    }
  }
  if (xd->left_available) {
    for (i = 0; i < block_size; ++i) {
      int diff = abs(*(rec_y - 1 + i * rec_ystride) -
                     *(tmp_y + i * tmp_ystride));
#ifdef SEARCH_MIN3
      // Searches for the min abs diff among 3 pixel neighbors in the border
      int diff1 = xd->up_available ?
          abs(*(rec_y - 1 + (i - 1) * rec_ystride) -
                      *(tmp_y + i * tmp_ystride)) : diff;
      int diff2 = i < block_size - 1 ?
          abs(*(rec_y - 1 + (i + 1) * rec_ystride) -
              *(tmp_y + i * tmp_ystride)) : diff;
      diff = diff <= diff1 ? diff : diff1;
      diff = diff <= diff2 ? diff : diff2;
#endif
      metric += diff;
    }
  }
  return metric;
}

static int get_weight(MACROBLOCKD *xd, int64_t metric_1, int64_t metric_2) {
  int weight = AVERAGE_WEIGHT;
  if (2 * metric_1 < metric_2)
    weight = 6;
  else if (4 * metric_1 < 3 * metric_2)
    weight = 5;
  else if (2 * metric_2 < metric_1)
    weight = 2;
  else if (4 * metric_2 < 3 * metric_1)
    weight = 3;
  return weight;
}

#ifdef USE_IMPLICIT_WEIGHT_SPLITMV
static int get_implicit_compoundinter_weight_splitmv(
    MACROBLOCKD *xd, int mb_row, int mb_col) {
  MB_MODE_INFO *mbmi = &xd->mode_info_context->mbmi;
  BLOCKD *blockd = xd->block;
  const int use_second_ref = mbmi->second_ref_frame > 0;
  int64_t metric_2 = 0, metric_1 = 0;
  int i, which_mv, weight;
  uint8_t tmp_y[256];
  const int tmp_ystride = 16;

  if (!use_second_ref) return 0;
  if (!(xd->up_available || xd->left_available))
    return AVERAGE_WEIGHT;

  assert(xd->mode_info_context->mbmi.mode == SPLITMV);

  which_mv = 1;  // second predictor
  if (xd->mode_info_context->mbmi.partitioning != PARTITIONING_4X4) {
    for (i = 0; i < 16; i += 8) {
      BLOCKD *d0 = &blockd[i];
      BLOCKD *d1 = &blockd[i + 2];
      const int y = i & 8;

      blockd[i + 0].bmi = xd->mode_info_context->bmi[i + 0];
      blockd[i + 2].bmi = xd->mode_info_context->bmi[i + 2];

      if (mbmi->need_to_clamp_mvs) {
        clamp_mv_to_umv_border(&blockd[i + 0].bmi.as_mv[which_mv].as_mv, xd);
        clamp_mv_to_umv_border(&blockd[i + 2].bmi.as_mv[which_mv].as_mv, xd);
      }
      if (i == 0) {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 8, 16,
                                     which_mv, 0, 16, 1,
                                     &xd->subpix, mb_row * 16 + y, mb_col * 16);
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 8, 16,
                                     which_mv, 0, 1, 8,
                                     &xd->subpix, mb_row * 16 + y, mb_col * 16);
      } else {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y + 8 * 16,
                                     8, 16, which_mv, 0, 1, 8,
                                     &xd->subpix, mb_row * 16 + y, mb_col * 16);
      }
    }
  } else {
    for (i = 0; i < 16; i += 2) {
      BLOCKD *d0 = &blockd[i];
      BLOCKD *d1 = &blockd[i + 1];
      const int x = (i & 3) * 4;
      const int y = (i >> 2) * 4;

      blockd[i + 0].bmi = xd->mode_info_context->bmi[i + 0];
      blockd[i + 1].bmi = xd->mode_info_context->bmi[i + 1];

      if (i >= 4 && (i & 3) != 0) continue;

      if (i == 0) {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 4, 16,
                                     which_mv, 0, 8, 1, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 4, 16,
                                     which_mv, 0, 1, 4, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
      } else if (i < 4) {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y + x, 4, 16,
                                     which_mv, 0, 8, 1, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
      } else {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y + y * 16,
                                     4, 16, which_mv, 0, 1, 4, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
      }
    }
  }
  metric_2 = get_consistency_metric(xd, tmp_y, tmp_ystride);

  which_mv = 0;  // first predictor
  if (xd->mode_info_context->mbmi.partitioning != PARTITIONING_4X4) {
    for (i = 0; i < 16; i += 8) {
      BLOCKD *d0 = &blockd[i];
      BLOCKD *d1 = &blockd[i + 2];
      const int y = i & 8;

      blockd[i + 0].bmi = xd->mode_info_context->bmi[i + 0];
      blockd[i + 2].bmi = xd->mode_info_context->bmi[i + 2];

      if (mbmi->need_to_clamp_mvs) {
        clamp_mv_to_umv_border(&blockd[i + 0].bmi.as_mv[which_mv].as_mv, xd);
        clamp_mv_to_umv_border(&blockd[i + 2].bmi.as_mv[which_mv].as_mv, xd);
      }
      if (i == 0) {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 8, 16,
                                     which_mv, 0, 16, 1,
                                     &xd->subpix, mb_row * 16 + y, mb_col * 16);
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 8, 16,
                                     which_mv, 0, 1, 8,
                                     &xd->subpix, mb_row * 16 + y, mb_col * 16);
      } else {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y + 8 * 16,
                                     8, 16, which_mv, 0, 1, 8,
                                     &xd->subpix, mb_row * 16 + y, mb_col * 16);
      }
    }
  } else {
    for (i = 0; i < 16; i += 2) {
      BLOCKD *d0 = &blockd[i];
      BLOCKD *d1 = &blockd[i + 1];
      const int x = (i & 3) * 4;
      const int y = (i >> 2) * 4;

      blockd[i + 0].bmi = xd->mode_info_context->bmi[i + 0];
      blockd[i + 1].bmi = xd->mode_info_context->bmi[i + 1];

      if (i >= 4 && (i & 3) != 0) continue;

      if (i == 0) {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 4, 16,
                                     which_mv, 0, 8, 1, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y, 4, 16,
                                     which_mv, 0, 1, 4, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
      } else if (i < 4) {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y + x, 4, 16,
                                     which_mv, 0, 8, 1, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
      } else {
        build_2x1_inter_predictor_wh(d0, d1, xd->scale_factor, tmp_y + y * 16,
                                     4, 16, which_mv, 0, 1, 4, &xd->subpix,
                                     mb_row * 16 + y, mb_col * 16 + x);
      }
    }
  }
  metric_1 = get_consistency_metric(xd, tmp_y, tmp_ystride);

  // Choose final weight for averaging
  weight = get_weight(xd, metric_1, metric_2);
  return weight;
}
#endif

static int get_implicit_compoundinter_weight(MACROBLOCKD *xd,
                                             int mb_row,
                                             int mb_col) {
  const int use_second_ref = xd->mode_info_context->mbmi.second_ref_frame > 0;
  int64_t metric_2 = 0, metric_1 = 0;
  int n, clamp_mvs, pre_stride;
  uint8_t *base_pre;
  int_mv ymv;
  uint8_t tmp_y[4096];
  const int tmp_ystride = 64;
  int weight;
  int edge[4];
  int block_size = 16 <<  xd->mode_info_context->mbmi.sb_type;
  struct scale_factors *scale;

  if (!use_second_ref) return 0;
  if (!(xd->up_available || xd->left_available))
    return AVERAGE_WEIGHT;

  edge[0] = xd->mb_to_top_edge;
  edge[1] = xd->mb_to_bottom_edge;
  edge[2] = xd->mb_to_left_edge;
  edge[3] = xd->mb_to_right_edge;

  clamp_mvs = xd->mode_info_context->mbmi.need_to_clamp_secondmv;
  base_pre = xd->second_pre.y_buffer;
  pre_stride = xd->second_pre.y_stride;
  ymv.as_int = xd->mode_info_context->mbmi.mv[1].as_int;
  // First generate the second predictor
  scale = &xd->scale_factor[1];
  for (n = 0; n < block_size; n += 16) {
    xd->mb_to_left_edge   = edge[2] - (n << 3);
    xd->mb_to_right_edge  = edge[3] + ((16 - n) << 3);
    if (clamp_mvs)
      clamp_mv_to_umv_border(&ymv.as_mv, xd);
    scale->set_scaled_offsets(scale, mb_row * 16, mb_col * 16 + n);
    // predict a single row of pixels
    vp9_build_inter_predictor(base_pre +
        scaled_buffer_offset(n, 0, pre_stride, scale),
        pre_stride, tmp_y + n, tmp_ystride, &ymv, scale, 16, 1, 0, &xd->subpix);
  }
  xd->mb_to_left_edge = edge[2];
  xd->mb_to_right_edge = edge[3];
  for (n = 0; n < block_size; n += 16) {
    xd->mb_to_top_edge    = edge[0] - (n << 3);
    xd->mb_to_bottom_edge = edge[1] + ((16 - n) << 3);
    if (clamp_mvs)
      clamp_mv_to_umv_border(&ymv.as_mv, xd);
    scale->set_scaled_offsets(scale, mb_row * 16 + n, mb_col * 16);
    // predict a single col of pixels
    vp9_build_inter_predictor(base_pre +
        scaled_buffer_offset(0, n, pre_stride, scale),
        pre_stride, tmp_y + n * tmp_ystride, tmp_ystride, &ymv,
        scale, 1, 16, 0, &xd->subpix);
  }
  xd->mb_to_top_edge = edge[0];
  xd->mb_to_bottom_edge = edge[1];
  // Compute consistency metric
  metric_2 = get_consistency_metric(xd, tmp_y, tmp_ystride);

  clamp_mvs = xd->mode_info_context->mbmi.need_to_clamp_mvs;
  base_pre = xd->pre.y_buffer;
  pre_stride = xd->pre.y_stride;
  ymv.as_int = xd->mode_info_context->mbmi.mv[0].as_int;
  // Now generate the first predictor
  scale = &xd->scale_factor[0];
  for (n = 0; n < block_size; n += 16) {
    xd->mb_to_left_edge   = edge[2] - (n << 3);
    xd->mb_to_right_edge  = edge[3] + ((16 - n) << 3);
    if (clamp_mvs)
      clamp_mv_to_umv_border(&ymv.as_mv, xd);
    scale->set_scaled_offsets(scale, mb_row * 16, mb_col * 16 + n);
    // predict a single row of pixels
    vp9_build_inter_predictor(base_pre +
        scaled_buffer_offset(n, 0, pre_stride, scale),
        pre_stride, tmp_y + n, tmp_ystride, &ymv, scale, 16, 1, 0, &xd->subpix);
  }
  xd->mb_to_left_edge = edge[2];
  xd->mb_to_right_edge = edge[3];
  for (n = 0; n < block_size; n += 16) {
    xd->mb_to_top_edge    = edge[0] - (n << 3);
    xd->mb_to_bottom_edge = edge[1] + ((16 - n) << 3);
    if (clamp_mvs)
      clamp_mv_to_umv_border(&ymv.as_mv, xd);
    scale->set_scaled_offsets(scale, mb_row * 16 + n, mb_col * 16);
    // predict a single col of pixels
    vp9_build_inter_predictor(base_pre +
        scaled_buffer_offset(0, n, pre_stride, scale),
        pre_stride, tmp_y + n * tmp_ystride, tmp_ystride, &ymv,
        scale, 1, 16, 0, &xd->subpix);
  }
  xd->mb_to_top_edge = edge[0];
  xd->mb_to_bottom_edge = edge[1];
  metric_1 = get_consistency_metric(xd, tmp_y, tmp_ystride);

  // Choose final weight for averaging
  weight = get_weight(xd, metric_1, metric_2);
  return weight;
}

static void build_inter16x16_predictors_mby_w(MACROBLOCKD *xd,
                                              uint8_t *dst_y,
                                              int dst_ystride,
                                              int weight,
                                              int mb_row,
                                              int mb_col) {
  const int use_second_ref = xd->mode_info_context->mbmi.second_ref_frame > 0;
  int which_mv;

  for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
    const int clamp_mvs = which_mv ?
        xd->mode_info_context->mbmi.need_to_clamp_secondmv :
         xd->mode_info_context->mbmi.need_to_clamp_mvs;

    uint8_t *base_pre = which_mv ? xd->second_pre.y_buffer : xd->pre.y_buffer;
    int pre_stride = which_mv ? xd->second_pre.y_stride : xd->pre.y_stride;
    int_mv ymv;
    struct scale_factors *scale = &xd->scale_factor[which_mv];

    ymv.as_int = xd->mode_info_context->mbmi.mv[which_mv].as_int;

    if (clamp_mvs)
      clamp_mv_to_umv_border(&ymv.as_mv, xd);

    scale->set_scaled_offsets(scale, mb_row * 16, mb_col * 16);

    vp9_build_inter_predictor(base_pre, pre_stride, dst_y, dst_ystride,
                              &ymv, scale, 16, 16,
                              which_mv ? weight : 0, &xd->subpix);
  }
}
#endif

#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT
static void build_inter16x16_predictors_mbuv_w(MACROBLOCKD *xd,
                                               uint8_t *dst_u,
                                               uint8_t *dst_v,
                                               int dst_uvstride,
                                               int weight,
                                               int mb_row,
                                               int mb_col) {
  const int use_second_ref = xd->mode_info_context->mbmi.second_ref_frame > 0;
  int which_mv;

  for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
    const int clamp_mvs =
        which_mv ? xd->mode_info_context->mbmi.need_to_clamp_secondmv
                 : xd->mode_info_context->mbmi.need_to_clamp_mvs;
    uint8_t *uptr, *vptr;
    int pre_stride = which_mv ? xd->second_pre.uv_stride
                              : xd->pre.uv_stride;
    int_mv mv;

    struct scale_factors *scale = &xd->scale_factor_uv[which_mv];
    mv.as_int = xd->mode_info_context->mbmi.mv[which_mv].as_int;


    if (clamp_mvs)
      clamp_mv_to_umv_border(&mv.as_mv, xd);

    uptr = (which_mv ? xd->second_pre.u_buffer : xd->pre.u_buffer);
    vptr = (which_mv ? xd->second_pre.v_buffer : xd->pre.v_buffer);

    scale->set_scaled_offsets(scale, mb_row * 16, mb_col * 16);

    vp9_build_inter_predictor_q4(
        uptr, pre_stride, dst_u, dst_uvstride, &mv,
        scale, 8, 8, which_mv ? weight : 0, &xd->subpix);

    vp9_build_inter_predictor_q4(
        vptr, pre_stride, dst_v, dst_uvstride, &mv,
        scale, 8, 8, which_mv ? weight : 0, &xd->subpix);
  }
}

void vp9_build_inter16x16_predictors_mbuv(MACROBLOCKD *xd,
                                          uint8_t *dst_u,
                                          uint8_t *dst_v,
                                          int dst_uvstride,
                                          int mb_row,
                                          int mb_col) {
#ifdef USE_IMPLICIT_WEIGHT_UV
  int weight = get_implicit_compoundinter_weight(xd, mb_row, mb_col);
#else
  int weight = AVERAGE_WEIGHT;
#endif
  build_inter16x16_predictors_mbuv_w(xd, dst_u, dst_v, dst_uvstride,
                                     weight, mb_row, mb_col);
}

#else

void vp9_build_inter16x16_predictors_mbuv(MACROBLOCKD *xd,
                                          uint8_t *dst_u,
                                          uint8_t *dst_v,
                                          int dst_uvstride,
                                          int mb_row,
                                          int mb_col) {
  const int use_second_ref = xd->mode_info_context->mbmi.second_ref_frame > 0;
  int which_mv;

  for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
    const int clamp_mvs =
        which_mv ? xd->mode_info_context->mbmi.need_to_clamp_secondmv
                 : xd->mode_info_context->mbmi.need_to_clamp_mvs;
    uint8_t *uptr, *vptr;
    int pre_stride = which_mv ? xd->second_pre.uv_stride
                              : xd->pre.uv_stride;
    int_mv mv;

    struct scale_factors *scale = &xd->scale_factor_uv[which_mv];
    mv.as_int = xd->mode_info_context->mbmi.mv[which_mv].as_int;


    if (clamp_mvs)
      clamp_mv_to_umv_border(&mv.as_mv, xd);

    uptr = (which_mv ? xd->second_pre.u_buffer : xd->pre.u_buffer);
    vptr = (which_mv ? xd->second_pre.v_buffer : xd->pre.v_buffer);

    scale->set_scaled_offsets(scale, mb_row * 16, mb_col * 16);

    vp9_build_inter_predictor_q4(
        uptr, pre_stride, dst_u, dst_uvstride, &mv,
        scale, 8, 8,
        which_mv << (2 * CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT), &xd->subpix);

    vp9_build_inter_predictor_q4(
        vptr, pre_stride, dst_v, dst_uvstride, &mv,
        scale, 8, 8,
        which_mv << (2 * CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT), &xd->subpix);
  }
}
#endif

#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT
static void build_inter_predictors_sby_w(MACROBLOCKD *x,
                                         uint8_t *dst_y,
                                         int dst_ystride,
                                         int weight,
                                         int mb_row,
                                         int mb_col,
                                         BLOCK_SIZE_TYPE bsize) {
  const int bwl = mb_width_log2(bsize),  bw = 1 << bwl;
  const int bhl = mb_height_log2(bsize), bh = 1 << bhl;
  uint8_t *y1 = x->pre.y_buffer;
  uint8_t *y2 = x->second_pre.y_buffer;
  int edge[4], n;

  edge[0] = x->mb_to_top_edge;
  edge[1] = x->mb_to_bottom_edge;
  edge[2] = x->mb_to_left_edge;
  edge[3] = x->mb_to_right_edge;

  for (n = 0; n < bw * bh; n++) {
    const int x_idx = n & (bw - 1), y_idx = n >> bwl;

    x->mb_to_top_edge    = edge[0] -           ((y_idx  * 16) << 3);
    x->mb_to_bottom_edge = edge[1] + (((bh - 1 - y_idx) * 16) << 3);
    x->mb_to_left_edge   = edge[2] -           ((x_idx  * 16) << 3);
    x->mb_to_right_edge  = edge[3] + (((bw - 1 - x_idx) * 16) << 3);

    x->pre.y_buffer = y1 + scaled_buffer_offset(x_idx * 16,
                                                y_idx * 16,
                                                x->pre.y_stride,
                                                &x->scale_factor[0]);
    if (x->mode_info_context->mbmi.second_ref_frame > 0) {
      x->second_pre.y_buffer = y2 +
          scaled_buffer_offset(x_idx * 16,
                               y_idx * 16,
                               x->second_pre.y_stride,
                               &x->scale_factor[1]);
    }
    build_inter16x16_predictors_mby_w(x,
        dst_y + y_idx * 16 * dst_ystride  + x_idx * 16,
        dst_ystride, weight, mb_row + y_idx, mb_col + x_idx);
  }
  x->mb_to_top_edge    = edge[0];
  x->mb_to_bottom_edge = edge[1];
  x->mb_to_left_edge   = edge[2];
  x->mb_to_right_edge  = edge[3];

  x->pre.y_buffer = y1;
  if (x->mode_info_context->mbmi.second_ref_frame > 0) {
    x->second_pre.y_buffer = y2;
  }
}

void vp9_build_inter_predictors_sby(MACROBLOCKD *x,
                                         uint8_t *dst_y,
                                         int dst_ystride,
                                         int mb_row,
                                         int mb_col,
                                         BLOCK_SIZE_TYPE bsize) {
  int weight = get_implicit_compoundinter_weight(x, mb_row, mb_col);
  build_inter_predictors_sby_w(x, dst_y, dst_ystride, weight,
                                    mb_row, mb_col, bsize);
}
#endif

#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT
static void build_inter_predictors_sbuv_w(MACROBLOCKD *x,
                                          uint8_t *dst_u,
                                          uint8_t *dst_v,
                                          int dst_uvstride,
                                          int weight,
                                          int mb_row,
                                          int mb_col,
                                          BLOCK_SIZE_TYPE bsize) {
  const int bwl = mb_width_log2(bsize),  bw = 1 << bwl;
  const int bhl = mb_height_log2(bsize), bh = 1 << bhl;
  uint8_t *u1 = x->pre.u_buffer, *v1 = x->pre.v_buffer;
  uint8_t *u2 = x->second_pre.u_buffer, *v2 = x->second_pre.v_buffer;
  int edge[4], n;

  edge[0] = x->mb_to_top_edge;
  edge[1] = x->mb_to_bottom_edge;
  edge[2] = x->mb_to_left_edge;
  edge[3] = x->mb_to_right_edge;

  for (n = 0; n < bw * bh; n++) {
    int scaled_uv_offset;
    const int x_idx = n & (bw - 1), y_idx = n >> bwl;

    x->mb_to_top_edge    = edge[0] -           ((y_idx  * 16) << 3);
    x->mb_to_bottom_edge = edge[1] + (((bh - 1 - y_idx) * 16) << 3);
    x->mb_to_left_edge   = edge[2] -           ((x_idx  * 16) << 3);
    x->mb_to_right_edge  = edge[3] + (((bw - 1 - x_idx) * 16) << 3);

    scaled_uv_offset = scaled_buffer_offset(x_idx * 8,
                                            y_idx * 8,
                                            x->pre.uv_stride,
                                            &x->scale_factor_uv[0]);
    x->pre.u_buffer = u1 + scaled_uv_offset;
    x->pre.v_buffer = v1 + scaled_uv_offset;

    if (x->mode_info_context->mbmi.second_ref_frame > 0) {
      scaled_uv_offset = scaled_buffer_offset(x_idx * 8,
                                              y_idx * 8,
                                              x->second_pre.uv_stride,
                                              &x->scale_factor_uv[1]);
      x->second_pre.u_buffer = u2 + scaled_uv_offset;
      x->second_pre.v_buffer = v2 + scaled_uv_offset;
    }

    build_inter16x16_predictors_mbuv_w(x,
        dst_u + y_idx *  8 * dst_uvstride + x_idx *  8,
        dst_v + y_idx *  8 * dst_uvstride + x_idx *  8,
        dst_uvstride, weight, mb_row + y_idx, mb_col + x_idx);
  }
  x->mb_to_top_edge    = edge[0];
  x->mb_to_bottom_edge = edge[1];
  x->mb_to_left_edge   = edge[2];
  x->mb_to_right_edge  = edge[3];

  x->pre.u_buffer = u1;
  x->pre.v_buffer = v1;

  if (x->mode_info_context->mbmi.second_ref_frame > 0) {
    x->second_pre.u_buffer = u2;
    x->second_pre.v_buffer = v2;
  }
}

void vp9_build_inter_predictors_sbuv(MACROBLOCKD *xd,
                                     uint8_t *dst_u,
                                     uint8_t *dst_v,
                                     int dst_uvstride,
                                     int mb_row,
                                     int mb_col,
                                     BLOCK_SIZE_TYPE bsize) {
#ifdef USE_IMPLICIT_WEIGHT_UV
  int weight = get_implicit_compoundinter_weight(xd, mb_row, mb_col);
#else
  int weight = AVERAGE_WEIGHT;
#endif
  build_inter_predictors_sbuv_w(xd, dst_u, dst_v, dst_uvstride,
                                weight, mb_row, mb_col, bsize);
}

#else

void vp9_build_inter_predictors_sbuv(MACROBLOCKD *x,
                                     uint8_t *dst_u,
                                     uint8_t *dst_v,
                                     int dst_uvstride,
                                     int mb_row,
                                     int mb_col,
                                     BLOCK_SIZE_TYPE bsize) {
  const int bwl = mb_width_log2(bsize),  bw = 1 << bwl;
  const int bhl = mb_height_log2(bsize), bh = 1 << bhl;
  uint8_t *u1 = x->pre.u_buffer, *v1 = x->pre.v_buffer;
  uint8_t *u2 = x->second_pre.u_buffer, *v2 = x->second_pre.v_buffer;
  int edge[4], n;

  edge[0] = x->mb_to_top_edge;
  edge[1] = x->mb_to_bottom_edge;
  edge[2] = x->mb_to_left_edge;
  edge[3] = x->mb_to_right_edge;

  for (n = 0; n < bw * bh; n++) {
    int scaled_uv_offset;
    const int x_idx = n & (bw - 1), y_idx = n >> bwl;

    x->mb_to_top_edge    = edge[0] -           ((y_idx  * 16) << 3);
    x->mb_to_bottom_edge = edge[1] + (((bh - 1 - y_idx) * 16) << 3);
    x->mb_to_left_edge   = edge[2] -           ((x_idx  * 16) << 3);
    x->mb_to_right_edge  = edge[3] + (((bw - 1 - x_idx) * 16) << 3);

    scaled_uv_offset = scaled_buffer_offset(x_idx * 8,
                                            y_idx * 8,
                                            x->pre.uv_stride,
                                            &x->scale_factor_uv[0]);
    x->pre.u_buffer = u1 + scaled_uv_offset;
    x->pre.v_buffer = v1 + scaled_uv_offset;

    if (x->mode_info_context->mbmi.second_ref_frame > 0) {
      scaled_uv_offset = scaled_buffer_offset(x_idx * 8,
                                              y_idx * 8,
                                              x->second_pre.uv_stride,
                                              &x->scale_factor_uv[1]);
      x->second_pre.u_buffer = u2 + scaled_uv_offset;
      x->second_pre.v_buffer = v2 + scaled_uv_offset;
    }

    vp9_build_inter16x16_predictors_mbuv(x,
        dst_u + y_idx *  8 * dst_uvstride + x_idx *  8,
        dst_v + y_idx *  8 * dst_uvstride + x_idx *  8,
        dst_uvstride, mb_row + y_idx, mb_col + x_idx);
  }
  x->mb_to_top_edge    = edge[0];
  x->mb_to_bottom_edge = edge[1];
  x->mb_to_left_edge   = edge[2];
  x->mb_to_right_edge  = edge[3];

  x->pre.u_buffer = u1;
  x->pre.v_buffer = v1;

  if (x->mode_info_context->mbmi.second_ref_frame > 0) {
    x->second_pre.u_buffer = u2;
    x->second_pre.v_buffer = v2;
  }
}
#endif

void vp9_build_inter_predictors_sb(MACROBLOCKD *mb,
                                   int mb_row, int mb_col,
                                   BLOCK_SIZE_TYPE bsize) {
  uint8_t *const y = mb->dst.y_buffer;
  uint8_t *const u = mb->dst.u_buffer;
  uint8_t *const v = mb->dst.v_buffer;
  const int y_stride = mb->dst.y_stride;
  const int uv_stride = mb->dst.uv_stride;

  vp9_build_inter_predictors_sby(mb, y, y_stride, mb_row, mb_col, bsize);
  vp9_build_inter_predictors_sbuv(mb, u, v, uv_stride, mb_row, mb_col, bsize);
#if CONFIG_COMP_INTERINTRA_PRED
  if (mb->mode_info_context->mbmi.second_ref_frame == INTRA_FRAME) {
    if (bsize == BLOCK_SIZE_SB32X32)
      vp9_build_interintra_32x32_predictors_sb(mb, y, u, v,
                                               y_stride, uv_stride);
    else
      vp9_build_interintra_64x64_predictors_sb(mb, y, u, v,
                                               y_stride, uv_stride);
  }
#endif
}

static void build_inter4x4_predictors_mb(MACROBLOCKD *xd,
                                         int mb_row, int mb_col) {
  int i;
  MB_MODE_INFO * mbmi = &xd->mode_info_context->mbmi;
  BLOCKD *blockd = xd->block;
  int which_mv = 0;
  const int use_second_ref = mbmi->second_ref_frame > 0;
#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT && defined(USE_IMPLICIT_WEIGHT_SPLITMV)
  int weight = get_implicit_compoundinter_weight_splitmv(xd, mb_row, mb_col);
#else
  int weight = AVERAGE_WEIGHT;
#endif

  if (xd->mode_info_context->mbmi.partitioning != PARTITIONING_4X4) {
    for (i = 0; i < 16; i += 8) {
      BLOCKD *d0 = &blockd[i];
      BLOCKD *d1 = &blockd[i + 2];
      const int y = i & 8;

      blockd[i + 0].bmi = xd->mode_info_context->bmi[i + 0];
      blockd[i + 2].bmi = xd->mode_info_context->bmi[i + 2];

      for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
        if (mbmi->need_to_clamp_mvs) {
          clamp_mv_to_umv_border(&blockd[i + 0].bmi.as_mv[which_mv].as_mv, xd);
          clamp_mv_to_umv_border(&blockd[i + 2].bmi.as_mv[which_mv].as_mv, xd);
        }

        build_2x1_inter_predictor(d0, d1, xd->scale_factor, 8, 16, which_mv,
                                  which_mv ? weight : 0,
                                  &xd->subpix, mb_row * 16 + y, mb_col * 16);
      }
    }
  } else {
    for (i = 0; i < 16; i += 2) {
      BLOCKD *d0 = &blockd[i];
      BLOCKD *d1 = &blockd[i + 1];
      const int x = (i & 3) * 4;
      const int y = (i >> 2) * 4;

      blockd[i + 0].bmi = xd->mode_info_context->bmi[i + 0];
      blockd[i + 1].bmi = xd->mode_info_context->bmi[i + 1];

      for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
        build_2x1_inter_predictor(d0, d1, xd->scale_factor, 4, 16, which_mv,
                                  which_mv ? weight : 0,
                                  &xd->subpix,
                                  mb_row * 16 + y, mb_col * 16 + x);
      }
    }
  }
#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT
#if !defined(USE_IMPLICIT_WEIGHT_UV)
  weight = AVERAGE_WEIGHT;
#endif
#endif
  for (i = 16; i < 24; i += 2) {
    BLOCKD *d0 = &blockd[i];
    BLOCKD *d1 = &blockd[i + 1];
    const int x = 4 * (i & 1);
    const int y = ((i - 16) >> 1) * 4;

    for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
      build_2x1_inter_predictor(d0, d1, xd->scale_factor_uv, 4, 8, which_mv,
                                which_mv ? weight : 0, &xd->subpix,
                                mb_row * 8 + y, mb_col * 8 + x);
    }
  }
}

static INLINE int round_mv_comp(int value) {
  return (value < 0 ? value - 4 : value + 4) / 8;
}

static int mi_mv_pred_row(MACROBLOCKD *mb, int off, int idx) {
  const int temp = mb->mode_info_context->bmi[off + 0].as_mv[idx].as_mv.row +
                   mb->mode_info_context->bmi[off + 1].as_mv[idx].as_mv.row +
                   mb->mode_info_context->bmi[off + 4].as_mv[idx].as_mv.row +
                   mb->mode_info_context->bmi[off + 5].as_mv[idx].as_mv.row;
  return round_mv_comp(temp);
}

static int mi_mv_pred_col(MACROBLOCKD *mb, int off, int idx) {
  const int temp = mb->mode_info_context->bmi[off + 0].as_mv[idx].as_mv.col +
                   mb->mode_info_context->bmi[off + 1].as_mv[idx].as_mv.col +
                   mb->mode_info_context->bmi[off + 4].as_mv[idx].as_mv.col +
                   mb->mode_info_context->bmi[off + 5].as_mv[idx].as_mv.col;
  return round_mv_comp(temp);
}

static int b_mv_pred_row(MACROBLOCKD *mb, int off, int idx) {
  BLOCKD *const blockd = mb->block;
  const int temp = blockd[off + 0].bmi.as_mv[idx].as_mv.row +
                   blockd[off + 1].bmi.as_mv[idx].as_mv.row +
                   blockd[off + 4].bmi.as_mv[idx].as_mv.row +
                   blockd[off + 5].bmi.as_mv[idx].as_mv.row;
  return round_mv_comp(temp);
}

static int b_mv_pred_col(MACROBLOCKD *mb, int off, int idx) {
  BLOCKD *const blockd = mb->block;
  const int temp = blockd[off + 0].bmi.as_mv[idx].as_mv.col +
                   blockd[off + 1].bmi.as_mv[idx].as_mv.col +
                   blockd[off + 4].bmi.as_mv[idx].as_mv.col +
                   blockd[off + 5].bmi.as_mv[idx].as_mv.col;
  return round_mv_comp(temp);
}


static void build_4x4uvmvs(MACROBLOCKD *xd) {
  int i, j;
  BLOCKD *blockd = xd->block;

  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      const int yoffset = i * 8 + j * 2;
      const int uoffset = 16 + i * 2 + j;
      const int voffset = 20 + i * 2 + j;

      MV *u = &blockd[uoffset].bmi.as_mv[0].as_mv;
      MV *v = &blockd[voffset].bmi.as_mv[0].as_mv;
      u->row = mi_mv_pred_row(xd, yoffset, 0);
      u->col = mi_mv_pred_col(xd, yoffset, 0);

      // if (x->mode_info_context->mbmi.need_to_clamp_mvs)
      clamp_uvmv_to_umv_border(u, xd);

      // if (x->mode_info_context->mbmi.need_to_clamp_mvs)
      clamp_uvmv_to_umv_border(u, xd);

      v->row = u->row;
      v->col = u->col;

      if (xd->mode_info_context->mbmi.second_ref_frame > 0) {
        u = &blockd[uoffset].bmi.as_mv[1].as_mv;
        v = &blockd[voffset].bmi.as_mv[1].as_mv;
        u->row = mi_mv_pred_row(xd, yoffset, 1);
        u->col = mi_mv_pred_col(xd, yoffset, 1);

        // if (mbmi->need_to_clamp_mvs)
        clamp_uvmv_to_umv_border(u, xd);

        // if (mbmi->need_to_clamp_mvs)
        clamp_uvmv_to_umv_border(u, xd);

        v->row = u->row;
        v->col = u->col;
      }
    }
  }
}

void vp9_build_inter_predictors_mb(MACROBLOCKD *xd,
                                   int mb_row,
                                   int mb_col) {
  if (xd->mode_info_context->mbmi.mode != SPLITMV) {
    vp9_build_inter_predictors_sb(xd, mb_row, mb_col, BLOCK_SIZE_MB16X16);
  } else {
    build_4x4uvmvs(xd);
    build_inter4x4_predictors_mb(xd, mb_row, mb_col);
  }
}

/*encoder only*/
void vp9_build_inter4x4_predictors_mbuv(MACROBLOCKD *xd,
                                        int mb_row, int mb_col) {
  int i, j, weight;
  BLOCKD *const blockd = xd->block;

  /* build uv mvs */
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      const int yoffset = i * 8 + j * 2;
      const int uoffset = 16 + i * 2 + j;
      const int voffset = 20 + i * 2 + j;

      MV *u = &blockd[uoffset].bmi.as_mv[0].as_mv;
      MV *v = &blockd[voffset].bmi.as_mv[0].as_mv;

      v->row = u->row = b_mv_pred_row(xd, yoffset, 0);
      v->col = u->col = b_mv_pred_col(xd, yoffset, 0);

      if (xd->mode_info_context->mbmi.second_ref_frame > 0) {
        u = &blockd[uoffset].bmi.as_mv[1].as_mv;
        v = &blockd[voffset].bmi.as_mv[1].as_mv;

        v->row = u->row = b_mv_pred_row(xd, yoffset, 1);
        v->col = u->col = b_mv_pred_col(xd, yoffset, 1);
      }
    }
  }

#if CONFIG_IMPLICIT_COMPOUNDINTER_WEIGHT && \
  defined(USE_IMPLICIT_WEIGHT_SPLITMV) && \
  defined(USE_IMPLICIT_WEIGHT_UV)
  weight = get_implicit_compoundinter_weight_splitmv(xd, mb_row, mb_col);
#else
  weight = AVERAGE_WEIGHT;
#endif
  for (i = 16; i < 24; i += 2) {
    const int use_second_ref = xd->mode_info_context->mbmi.second_ref_frame > 0;
    const int x = 4 * (i & 1);
    const int y = ((i - 16) >> 1) * 4;

    int which_mv;
    BLOCKD *d0 = &blockd[i];
    BLOCKD *d1 = &blockd[i + 1];

    for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
      build_2x1_inter_predictor(d0, d1, xd->scale_factor_uv, 4, 8, which_mv,
                                which_mv ? weight : 0,
                                &xd->subpix, mb_row * 8 + y, mb_col * 8 + x);
    }
  }
}
