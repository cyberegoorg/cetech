#ifndef CETECH_GFX_H
#define CETECH_GFX_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

/// Color RGB/alpha/depth write. When it's not specified write will be disabled.
#define CT_RENDER_STATE_WRITE_R                 UINT64_C(0x0000000000000001) //!< Enable R write.
#define CT_RENDER_STATE_WRITE_G                 UINT64_C(0x0000000000000002) //!< Enable G write.
#define CT_RENDER_STATE_WRITE_B                 UINT64_C(0x0000000000000004) //!< Enable B write.
#define CT_RENDER_STATE_WRITE_A                 UINT64_C(0x0000000000000008) //!< Enable alpha write.
#define CT_RENDER_STATE_WRITE_Z                 UINT64_C(0x0000004000000000) //!< Enable depth write.

/// Enable RGB write.
#define CT_RENDER_STATE_WRITE_RGB (0  \
            | CT_RENDER_STATE_WRITE_R \
            | CT_RENDER_STATE_WRITE_G \
            | CT_RENDER_STATE_WRITE_B \
            )

/// Write all channels mask.
#define CT_RENDER_STATE_WRITE_MASK (0   \
            | CT_RENDER_STATE_WRITE_RGB \
            | CT_RENDER_STATE_WRITE_A   \
            | CT_RENDER_STATE_WRITE_Z   \
            )

/// Depth test state. When `CT_RENDER_STATE_DEPTH_` is not specified depth test will be disabled.
#define CT_RENDER_STATE_DEPTH_TEST_LESS         UINT64_C(0x0000000000000010) //!< Enable depth test, less.
#define CT_RENDER_STATE_DEPTH_TEST_LEQUAL       UINT64_C(0x0000000000000020) //!< Enable depth test, less or equal.
#define CT_RENDER_STATE_DEPTH_TEST_EQUAL        UINT64_C(0x0000000000000030) //!< Enable depth test, equal.
#define CT_RENDER_STATE_DEPTH_TEST_GEQUAL       UINT64_C(0x0000000000000040) //!< Enable depth test, greater or equal.
#define CT_RENDER_STATE_DEPTH_TEST_GREATER      UINT64_C(0x0000000000000050) //!< Enable depth test, greater.
#define CT_RENDER_STATE_DEPTH_TEST_NOTEQUAL     UINT64_C(0x0000000000000060) //!< Enable depth test, not equal.
#define CT_RENDER_STATE_DEPTH_TEST_NEVER        UINT64_C(0x0000000000000070) //!< Enable depth test, never.
#define CT_RENDER_STATE_DEPTH_TEST_ALWAYS       UINT64_C(0x0000000000000080) //!< Enable depth test, always.
#define CT_RENDER_STATE_DEPTH_TEST_SHIFT        4                            //!< Depth test state bit shift.
#define CT_RENDER_STATE_DEPTH_TEST_MASK         UINT64_C(0x00000000000000f0) //!< Depth test state bit mask.

/// Use CT_RENDER_STATE_BLEND_FUNC(_src, _dst) or CT_RENDER_STATE_BLEND_FUNC_SEPARATE(_srcRGB, _dstRGB, _srcA, _dstA)
/// helper macros.
#define CT_RENDER_STATE_BLEND_ZERO              UINT64_C(0x0000000000001000) //!< 0, 0, 0, 0
#define CT_RENDER_STATE_BLEND_ONE               UINT64_C(0x0000000000002000) //!< 1, 1, 1, 1
#define CT_RENDER_STATE_BLEND_SRC_COLOR         UINT64_C(0x0000000000003000) //!< Rs, Gs, Bs, As
#define CT_RENDER_STATE_BLEND_INV_SRC_COLOR     UINT64_C(0x0000000000004000) //!< 1-Rs, 1-Gs, 1-Bs, 1-As
#define CT_RENDER_STATE_BLEND_SRC_ALPHA         UINT64_C(0x0000000000005000) //!< As, As, As, As
#define CT_RENDER_STATE_BLEND_INV_SRC_ALPHA     UINT64_C(0x0000000000006000) //!< 1-As, 1-As, 1-As, 1-As
#define CT_RENDER_STATE_BLEND_DST_ALPHA         UINT64_C(0x0000000000007000) //!< Ad, Ad, Ad, Ad
#define CT_RENDER_STATE_BLEND_INV_DST_ALPHA     UINT64_C(0x0000000000008000) //!< 1-Ad, 1-Ad, 1-Ad ,1-Ad
#define CT_RENDER_STATE_BLEND_DST_COLOR         UINT64_C(0x0000000000009000) //!< Rd, Gd, Bd, Ad
#define CT_RENDER_STATE_BLEND_INV_DST_COLOR     UINT64_C(0x000000000000a000) //!< 1-Rd, 1-Gd, 1-Bd, 1-Ad
#define CT_RENDER_STATE_BLEND_SRC_ALPHA_SAT     UINT64_C(0x000000000000b000) //!< f, f, f, 1; f = min(As, 1-Ad)
#define CT_RENDER_STATE_BLEND_FACTOR            UINT64_C(0x000000000000c000) //!< Blend factor
#define CT_RENDER_STATE_BLEND_INV_FACTOR        UINT64_C(0x000000000000d000) //!< 1-Blend factor
#define CT_RENDER_STATE_BLEND_SHIFT             12                           //!< Blend state bit shift.
#define CT_RENDER_STATE_BLEND_MASK              UINT64_C(0x000000000ffff000) //!< Blend state bit mask.

/// Use CT_RENDER_STATE_BLEND_EQUATION(_equation) or CT_RENDER_STATE_BLEND_EQUATION_SEPARATE(_equationRGB, _equationA)
/// helper macros.
#define CT_RENDER_STATE_BLEND_EQUATION_ADD      UINT64_C(0x0000000000000000) //!< Blend add: src + dst.
#define CT_RENDER_STATE_BLEND_EQUATION_SUB      UINT64_C(0x0000000010000000) //!< Blend subtract: src - dst.
#define CT_RENDER_STATE_BLEND_EQUATION_REVSUB   UINT64_C(0x0000000020000000) //!< Blend reverse subtract: dst - src.
#define CT_RENDER_STATE_BLEND_EQUATION_MIN      UINT64_C(0x0000000030000000) //!< Blend min: min(src, dst).
#define CT_RENDER_STATE_BLEND_EQUATION_MAX      UINT64_C(0x0000000040000000) //!< Blend max: max(src, dst).
#define CT_RENDER_STATE_BLEND_EQUATION_SHIFT    28                           //!< Blend equation bit shift.
#define CT_RENDER_STATE_BLEND_EQUATION_MASK     UINT64_C(0x00000003f0000000) //!< Blend equation bit mask.

#define CT_RENDER_STATE_BLEND_INDEPENDENT       UINT64_C(0x0000000400000000) //!< Enable blend independent.
#define CT_RENDER_STATE_BLEND_ALPHA_TO_COVERAGE UINT64_C(0x0000000800000000) //!< Enable alpha to coverage.

/// Cull state. When `CT_RENDER_STATE_CULL_*` is not specified culling will be disabled.
#define CT_RENDER_STATE_CULL_CW                 UINT64_C(0x0000001000000000) //!< Cull clockwise triangles.
#define CT_RENDER_STATE_CULL_CCW                UINT64_C(0x0000002000000000) //!< Cull counter-clockwise triangles.
#define CT_RENDER_STATE_CULL_SHIFT              36                           //!< Culling mode bit shift.
#define CT_RENDER_STATE_CULL_MASK               UINT64_C(0x0000003000000000) //!< Culling mode bit mask.

/// See CT_RENDER_STATE_ALPHA_REF(_ref) helper macro.
#define CT_RENDER_STATE_ALPHA_REF_SHIFT         40                           //!< Alpha reference bit shift.
#define CT_RENDER_STATE_ALPHA_REF_MASK          UINT64_C(0x0000ff0000000000) //!< Alpha reference bit mask.

#define CT_RENDER_STATE_PT_TRISTRIP             UINT64_C(0x0001000000000000) //!< Tristrip.
#define CT_RENDER_STATE_PT_LINES                UINT64_C(0x0002000000000000) //!< Lines.
#define CT_RENDER_STATE_PT_LINESTRIP            UINT64_C(0x0003000000000000) //!< Line strip.
#define CT_RENDER_STATE_PT_POINTS               UINT64_C(0x0004000000000000) //!< Points.
#define CT_RENDER_STATE_PT_SHIFT                48                           //!< Primitive type bit shift.
#define CT_RENDER_STATE_PT_MASK                 UINT64_C(0x0007000000000000) //!< Primitive type bit mask.

/// See CT_RENDER_STATE_POINT_SIZE(_size) helper macro.
#define CT_RENDER_STATE_POINT_SIZE_SHIFT        52                           //!< Point size bit shift.
#define CT_RENDER_STATE_POINT_SIZE_MASK         UINT64_C(0x00f0000000000000) //!< Point size bit mask.

/// Enable MSAA write when writing into MSAA frame buffer.
/// This flag is ignored when not writing into MSAA frame buffer.
#define CT_RENDER_STATE_MSAA                    UINT64_C(0x0100000000000000) //!< Enable MSAA rasterization.
#define CT_RENDER_STATE_LINEAA                  UINT64_C(0x0200000000000000) //!< Enable line AA rasterization.
#define CT_RENDER_STATE_CONSERVATIVE_RASTER     UINT64_C(0x0400000000000000) //!< Enable conservative rasterization.

/// Do not use!
#define CT_RENDER_STATE_RESERVED_SHIFT          61                           //!< Internal bits shift.
#define CT_RENDER_STATE_RESERVED_MASK           UINT64_C(0xe000000000000000) //!< Internal bits mask.

#define CT_RENDER_STATE_NONE                    UINT64_C(0x0000000000000000) //!< No state.
#define CT_RENDER_STATE_MASK                    UINT64_C(0xffffffffffffffff) //!< State mask.

/// Default state is write to RGB, alpha, and depth with depth test less enabled, with clockwise
/// culling and MSAA (when writing into MSAA frame buffer, otherwise this flag is ignored).
#define CT_RENDER_STATE_DEFAULT (0            \
            | CT_RENDER_STATE_WRITE_RGB       \
            | CT_RENDER_STATE_WRITE_A         \
            | CT_RENDER_STATE_WRITE_Z         \
            | CT_RENDER_STATE_DEPTH_TEST_LESS \
            | CT_RENDER_STATE_CULL_CW         \
            | CT_RENDER_STATE_MSAA            \
            )

/// Alpha reference value.
#define CT_RENDER_STATE_ALPHA_REF(_ref)   ( ( (uint64_t)(_ref )<<CT_RENDER_STATE_ALPHA_REF_SHIFT )&CT_RENDER_STATE_ALPHA_REF_MASK)

/// Point size value.
#define CT_RENDER_STATE_POINT_SIZE(_size) ( ( (uint64_t)(_size)<<CT_RENDER_STATE_POINT_SIZE_SHIFT)&CT_RENDER_STATE_POINT_SIZE_MASK)

/// Blend function separate.
#define CT_RENDER_STATE_BLEND_FUNC_SEPARATE(_srcRGB, _dstRGB, _srcA, _dstA) (UINT64_C(0) \
            | ( ( (uint64_t)(_srcRGB)|( (uint64_t)(_dstRGB)<<4) )   )               \
            | ( ( (uint64_t)(_srcA  )|( (uint64_t)(_dstA  )<<4) )<<8)               \
            )

/// Blend equation separate.
#define CT_RENDER_STATE_BLEND_EQUATION_SEPARATE(_equationRGB, _equationA) ( (uint64_t)(_equationRGB)|( (uint64_t)(_equationA)<<3) )

/// Blend function.
#define CT_RENDER_STATE_BLEND_FUNC(_src, _dst)    CT_RENDER_STATE_BLEND_FUNC_SEPARATE(_src, _dst, _src, _dst)

/// Blend equation.
#define CT_RENDER_STATE_BLEND_EQUATION(_equation) CT_RENDER_STATE_BLEND_EQUATION_SEPARATE(_equation, _equation)

/// Utility predefined blend modes.

/// Additive blending.
#define CT_RENDER_STATE_BLEND_ADD (0                                         \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_ONE, CT_RENDER_STATE_BLEND_ONE) \
    )

/// Alpha blend.
#define CT_RENDER_STATE_BLEND_ALPHA (0                                                       \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_SRC_ALPHA, CT_RENDER_STATE_BLEND_INV_SRC_ALPHA) \
    )

/// Selects darker color of blend.
#define CT_RENDER_STATE_BLEND_DARKEN (0                                      \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_ONE, CT_RENDER_STATE_BLEND_ONE) \
    | CT_RENDER_STATE_BLEND_EQUATION(CT_RENDER_STATE_BLEND_EQUATION_MIN)          \
    )

/// Selects lighter color of blend.
#define CT_RENDER_STATE_BLEND_LIGHTEN (0                                     \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_ONE, CT_RENDER_STATE_BLEND_ONE) \
    | CT_RENDER_STATE_BLEND_EQUATION(CT_RENDER_STATE_BLEND_EQUATION_MAX)          \
    )

/// Multiplies colors.
#define CT_RENDER_STATE_BLEND_MULTIPLY (0                                           \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_DST_COLOR, CT_RENDER_STATE_BLEND_ZERO) \
    )

/// Opaque pixels will cover the pixels directly below them without any math or algorithm applied to them.
#define CT_RENDER_STATE_BLEND_NORMAL (0                                                \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_ONE, CT_RENDER_STATE_BLEND_INV_SRC_ALPHA) \
    )

/// Multiplies the inverse of the blend and base colors.
#define CT_RENDER_STATE_BLEND_SCREEN (0                                                \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_ONE, CT_RENDER_STATE_BLEND_INV_SRC_COLOR) \
    )

/// Decreases the brightness of the base color based on the value of the blend color.
#define CT_RENDER_STATE_BLEND_LINEAR_BURN (0                                                 \
    | CT_RENDER_STATE_BLEND_FUNC(CT_RENDER_STATE_BLEND_DST_COLOR, CT_RENDER_STATE_BLEND_INV_DST_COLOR) \
    | CT_RENDER_STATE_BLEND_EQUATION(CT_RENDER_STATE_BLEND_EQUATION_SUB)                          \
    )

///
#define CT_RENDER_STATE_BLEND_FUNC_RT_x(_src, _dst) (0               \
            | ( (uint32_t)( (_src)>>CT_RENDER_STATE_BLEND_SHIFT)       \
            | ( (uint32_t)( (_dst)>>CT_RENDER_STATE_BLEND_SHIFT)<<4) ) \
            )

#define CT_RENDER_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation) (0               \
            | CT_RENDER_STATE_BLEND_FUNC_RT_x(_src, _dst)                        \
            | ( (uint32_t)( (_equation)>>CT_RENDER_STATE_BLEND_EQUATION_SHIFT)<<8) \
            )

#define CT_RENDER_STATE_BLEND_FUNC_RT_1(_src, _dst)  (CT_RENDER_STATE_BLEND_FUNC_RT_x(_src, _dst)<< 0)
#define CT_RENDER_STATE_BLEND_FUNC_RT_2(_src, _dst)  (CT_RENDER_STATE_BLEND_FUNC_RT_x(_src, _dst)<<11)
#define CT_RENDER_STATE_BLEND_FUNC_RT_3(_src, _dst)  (CT_RENDER_STATE_BLEND_FUNC_RT_x(_src, _dst)<<22)

#define CT_RENDER_STATE_BLEND_FUNC_RT_1E(_src, _dst, _equation) (CT_RENDER_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<< 0)
#define CT_RENDER_STATE_BLEND_FUNC_RT_2E(_src, _dst, _equation) (CT_RENDER_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<<11)
#define CT_RENDER_STATE_BLEND_FUNC_RT_3E(_src, _dst, _equation) (CT_RENDER_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<<22)

///
#define CT_RENDER_STENCIL_FUNC_REF_SHIFT      0                    //!<
#define CT_RENDER_STENCIL_FUNC_REF_MASK       UINT32_C(0x000000ff) //!<
#define CT_RENDER_STENCIL_FUNC_RMASK_SHIFT    8                    //!<
#define CT_RENDER_STENCIL_FUNC_RMASK_MASK     UINT32_C(0x0000ff00) //!<

#define CT_RENDER_STENCIL_TEST_LESS           UINT32_C(0x00010000) //!< Enable stencil test, less.
#define CT_RENDER_STENCIL_TEST_LEQUAL         UINT32_C(0x00020000) //!< Enable stencil test, less or equal.
#define CT_RENDER_STENCIL_TEST_EQUAL          UINT32_C(0x00030000) //!< Enable stencil test, equal.
#define CT_RENDER_STENCIL_TEST_GEQUAL         UINT32_C(0x00040000) //!< Enable stencil test, greater or equal.
#define CT_RENDER_STENCIL_TEST_GREATER        UINT32_C(0x00050000) //!< Enable stencil test, greater.
#define CT_RENDER_STENCIL_TEST_NOTEQUAL       UINT32_C(0x00060000) //!< Enable stencil test, not equal.
#define CT_RENDER_STENCIL_TEST_NEVER          UINT32_C(0x00070000) //!< Enable stencil test, never.
#define CT_RENDER_STENCIL_TEST_ALWAYS         UINT32_C(0x00080000) //!< Enable stencil test, always.
#define CT_RENDER_STENCIL_TEST_SHIFT          16                   //!< Stencil test bit shift.
#define CT_RENDER_STENCIL_TEST_MASK           UINT32_C(0x000f0000) //!< Stencil test bit mask.

#define CT_RENDER_STENCIL_OP_FAIL_S_ZERO      UINT32_C(0x00000000) //!< Zero.
#define CT_RENDER_STENCIL_OP_FAIL_S_KEEP      UINT32_C(0x00100000) //!< Keep.
#define CT_RENDER_STENCIL_OP_FAIL_S_REPLACE   UINT32_C(0x00200000) //!< Replace.
#define CT_RENDER_STENCIL_OP_FAIL_S_INCR      UINT32_C(0x00300000) //!< Increment and wrap.
#define CT_RENDER_STENCIL_OP_FAIL_S_INCRSAT   UINT32_C(0x00400000) //!< Increment and clamp.
#define CT_RENDER_STENCIL_OP_FAIL_S_DECR      UINT32_C(0x00500000) //!< Decrement and wrap.
#define CT_RENDER_STENCIL_OP_FAIL_S_DECRSAT   UINT32_C(0x00600000) //!< Decrement and clamp.
#define CT_RENDER_STENCIL_OP_FAIL_S_INVERT    UINT32_C(0x00700000) //!< Invert.
#define CT_RENDER_STENCIL_OP_FAIL_S_SHIFT     20                   //!< Stencil operation fail bit shift.
#define CT_RENDER_STENCIL_OP_FAIL_S_MASK      UINT32_C(0x00f00000) //!< Stencil operation fail bit mask.

#define CT_RENDER_STENCIL_OP_FAIL_Z_ZERO      UINT32_C(0x00000000) //!< Zero.
#define CT_RENDER_STENCIL_OP_FAIL_Z_KEEP      UINT32_C(0x01000000) //!< Keep.
#define CT_RENDER_STENCIL_OP_FAIL_Z_REPLACE   UINT32_C(0x02000000) //!< Replace.
#define CT_RENDER_STENCIL_OP_FAIL_Z_INCR      UINT32_C(0x03000000) //!< Increment and wrap.
#define CT_RENDER_STENCIL_OP_FAIL_Z_INCRSAT   UINT32_C(0x04000000) //!< Increment and clamp.
#define CT_RENDER_STENCIL_OP_FAIL_Z_DECR      UINT32_C(0x05000000) //!< Decrement and wrap.
#define CT_RENDER_STENCIL_OP_FAIL_Z_DECRSAT   UINT32_C(0x06000000) //!< Decrement and clamp.
#define CT_RENDER_STENCIL_OP_FAIL_Z_INVERT    UINT32_C(0x07000000) //!< Invert.
#define CT_RENDER_STENCIL_OP_FAIL_Z_SHIFT     24                   //!< Stencil operation depth fail bit shift
#define CT_RENDER_STENCIL_OP_FAIL_Z_MASK      UINT32_C(0x0f000000) //!< Stencil operation depth fail bit mask.

#define CT_RENDER_STENCIL_OP_PASS_Z_ZERO      UINT32_C(0x00000000) //!< Zero.
#define CT_RENDER_STENCIL_OP_PASS_Z_KEEP      UINT32_C(0x10000000) //!< Keep.
#define CT_RENDER_STENCIL_OP_PASS_Z_REPLACE   UINT32_C(0x20000000) //!< Replace.
#define CT_RENDER_STENCIL_OP_PASS_Z_INCR      UINT32_C(0x30000000) //!< Increment and wrap.
#define CT_RENDER_STENCIL_OP_PASS_Z_INCRSAT   UINT32_C(0x40000000) //!< Increment and clamp.
#define CT_RENDER_STENCIL_OP_PASS_Z_DECR      UINT32_C(0x50000000) //!< Decrement and wrap.
#define CT_RENDER_STENCIL_OP_PASS_Z_DECRSAT   UINT32_C(0x60000000) //!< Decrement and clamp.
#define CT_RENDER_STENCIL_OP_PASS_Z_INVERT    UINT32_C(0x70000000) //!< Invert.
#define CT_RENDER_STENCIL_OP_PASS_Z_SHIFT     28                   //!< Stencil operation depth pass bit shift
#define CT_RENDER_STENCIL_OP_PASS_Z_MASK      UINT32_C(0xf0000000) //!< Stencil operation depth pass bit mask.

#define CT_RENDER_STENCIL_NONE                UINT32_C(0x00000000) //!<
#define CT_RENDER_STENCIL_MASK                UINT32_C(0xffffffff) //!<
#define CT_RENDER_STENCIL_DEFAULT             UINT32_C(0x00000000) //!<

/// Set stencil ref value.
#define CT_RENDER_STENCIL_FUNC_REF(_ref) ( ((uint32_t)(_ref)<<CT_RENDER_STENCIL_FUNC_REF_SHIFT)&CT_RENDER_STENCIL_FUNC_REF_MASK)

/// Set stencil rmask value.
#define CT_RENDER_STENCIL_FUNC_RMASK(_mask) ( ((uint32_t)(_mask)<<CT_RENDER_STENCIL_FUNC_RMASK_SHIFT)&CT_RENDER_STENCIL_FUNC_RMASK_MASK)

///
#define CT_RENDER_CLEAR_NONE                  UINT16_C(0x0000) //!< No clear flags.
#define CT_RENDER_CLEAR_COLOR                 UINT16_C(0x0001) //!< Clear color.
#define CT_RENDER_CLEAR_DEPTH                 UINT16_C(0x0002) //!< Clear depth.
#define CT_RENDER_CLEAR_STENCIL               UINT16_C(0x0004) //!< Clear stencil.
#define CT_RENDER_CLEAR_DISCARD_COLOR_0       UINT16_C(0x0008) //!< Discard frame buffer attachment 0.
#define CT_RENDER_CLEAR_DISCARD_COLOR_1       UINT16_C(0x0010) //!< Discard frame buffer attachment 1.
#define CT_RENDER_CLEAR_DISCARD_COLOR_2       UINT16_C(0x0020) //!< Discard frame buffer attachment 2.
#define CT_RENDER_CLEAR_DISCARD_COLOR_3       UINT16_C(0x0040) //!< Discard frame buffer attachment 3.
#define CT_RENDER_CLEAR_DISCARD_COLOR_4       UINT16_C(0x0080) //!< Discard frame buffer attachment 4.
#define CT_RENDER_CLEAR_DISCARD_COLOR_5       UINT16_C(0x0100) //!< Discard frame buffer attachment 5.
#define CT_RENDER_CLEAR_DISCARD_COLOR_6       UINT16_C(0x0200) //!< Discard frame buffer attachment 6.
#define CT_RENDER_CLEAR_DISCARD_COLOR_7       UINT16_C(0x0400) //!< Discard frame buffer attachment 7.
#define CT_RENDER_CLEAR_DISCARD_DEPTH         UINT16_C(0x0800) //!< Discard frame buffer depth attachment.
#define CT_RENDER_CLEAR_DISCARD_STENCIL       UINT16_C(0x1000) //!< Discard frame buffer stencil attachment.

#define CT_RENDER_CLEAR_DISCARD_COLOR_MASK (0    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_0    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_1    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_2    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_3    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_4    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_5    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_6    \
            | CT_RENDER_CLEAR_DISCARD_COLOR_7    \
            )
#define CT_RENDER_CLEAR_DISCARD_MASK (0          \
            | CT_RENDER_CLEAR_DISCARD_COLOR_MASK \
            | CT_RENDER_CLEAR_DISCARD_DEPTH      \
            | CT_RENDER_CLEAR_DISCARD_STENCIL    \
            )

///
#define CT_RENDER_DEBUG_NONE                  UINT32_C(0x00000000) //!< No debug.
#define CT_RENDER_DEBUG_WIREFRAME             UINT32_C(0x00000001) //!< Enable wireframe for all primitives.
#define CT_RENDER_DEBUG_IFH                   UINT32_C(0x00000002) //!< Enable infinitely fast hardware test. No draw calls will be submitted to driver. It’s useful when profiling to quickly assess bottleneck between CPU and GPU.
#define CT_RENDER_DEBUG_STATS                 UINT32_C(0x00000004) //!< Enable statistics display.
#define CT_RENDER_DEBUG_TEXT                  UINT32_C(0x00000008) //!< Enable debug text display.
#define CT_RENDER_DEBUG_PROFILER              UINT32_C(0x00000010) //!< Enable profiler.

///
#define CT_RENDER_BUFFER_NONE                 UINT16_C(0x0000) //!<

#define CT_RENDER_BUFFER_COMPUTE_FORMAT_8x1   UINT16_C(0x0001) //!< 1 8-bit value
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_8x2   UINT16_C(0x0002) //!< 2 8-bit values
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_8x4   UINT16_C(0x0003) //!< 4 8-bit values
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_16x1  UINT16_C(0x0004) //!< 1 16-bit value
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_16x2  UINT16_C(0x0005) //!< 2 16-bit values
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_16x4  UINT16_C(0x0006) //!< 4 16-bit values
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_32x1  UINT16_C(0x0007) //!< 1 32-bit value
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_32x2  UINT16_C(0x0008) //!< 2 32-bit values
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_32x4  UINT16_C(0x0009) //!< 4 32-bit values
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_SHIFT 0                //!<
#define CT_RENDER_BUFFER_COMPUTE_FORMAT_MASK  UINT16_C(0x000f) //!<

#define CT_RENDER_BUFFER_COMPUTE_TYPE_INT     UINT16_C(0x0010) //!< Type `int`.
#define CT_RENDER_BUFFER_COMPUTE_TYPE_UINT    UINT16_C(0x0020) //!< Type `uint`.
#define CT_RENDER_BUFFER_COMPUTE_TYPE_FLOAT   UINT16_C(0x0030) //!< Type `float`.
#define CT_RENDER_BUFFER_COMPUTE_TYPE_SHIFT   4                //!<
#define CT_RENDER_BUFFER_COMPUTE_TYPE_MASK    UINT16_C(0x0030) //!<

#define CT_RENDER_BUFFER_COMPUTE_READ         UINT16_C(0x0100) //!< Buffer will be read by shader.
#define CT_RENDER_BUFFER_COMPUTE_WRITE        UINT16_C(0x0200) //!< Buffer will be used for writing.
#define CT_RENDER_BUFFER_DRAW_INDIRECT        UINT16_C(0x0400) //!< Buffer will be used for storing draw indirect commands.
#define CT_RENDER_BUFFER_ALLOW_RESIZE         UINT16_C(0x0800) //!< Allow dynamic index/vertex buffer resize during update.
#define CT_RENDER_BUFFER_INDEX32              UINT16_C(0x1000) //!< Index buffer contains 32-bit indices.

#define CT_RENDER_BUFFER_COMPUTE_READ_WRITE (0 \
            | CT_RENDER_BUFFER_COMPUTE_READ    \
            | CT_RENDER_BUFFER_COMPUTE_WRITE   \
            )

/// Texture creation flags.
#define CT_RENDER_TEXTURE_NONE                UINT64_C(0x0000000000000000) //!<
#define CT_RENDER_TEXTURE_MSAA_SAMPLE         UINT64_C(0x0000000800000000) //!< Texture will be used for MSAA sampling.
#define CT_RENDER_TEXTURE_RT                  UINT64_C(0x0000001000000000) //!< Render target no MSAA.
#define CT_RENDER_TEXTURE_RT_MSAA_X2          UINT64_C(0x0000002000000000) //!< Render target MSAAx2 mode.
#define CT_RENDER_TEXTURE_RT_MSAA_X4          UINT64_C(0x0000003000000000) //!< Render target MSAAx4 mode.
#define CT_RENDER_TEXTURE_RT_MSAA_X8          UINT64_C(0x0000004000000000) //!< Render target MSAAx8 mode.
#define CT_RENDER_TEXTURE_RT_MSAA_X16         UINT64_C(0x0000005000000000) //!< Render target MSAAx16 mode.
#define CT_RENDER_TEXTURE_RT_MSAA_SHIFT       36                           //!<
#define CT_RENDER_TEXTURE_RT_MSAA_MASK        UINT64_C(0x0000007000000000) //!<
#define CT_RENDER_TEXTURE_RT_WRITE_ONLY       UINT64_C(0x0000008000000000) //!< Render target will be used for writing only.
#define CT_RENDER_TEXTURE_RT_MASK             UINT64_C(0x000000f000000000) //!<
#define CT_RENDER_TEXTURE_COMPUTE_WRITE       UINT64_C(0x0000100000000000) //!< Texture will be used for compute write.
#define CT_RENDER_TEXTURE_SRGB                UINT64_C(0x0000200000000000) //!< Sample texture as sRGB.
#define CT_RENDER_TEXTURE_BLIT_DST            UINT64_C(0x0000400000000000) //!< Texture will be used as blit destination.
#define CT_RENDER_TEXTURE_READ_BACK           UINT64_C(0x0000800000000000) //!< Texture will be used for read back from GPU.

/// Sampler flags.
#define CT_RENDER_SAMPLER_NONE                UINT32_C(0x00000000) //!<
#define CT_RENDER_SAMPLER_U_MIRROR            UINT32_C(0x00000001) //!< Wrap U mode: Mirror
#define CT_RENDER_SAMPLER_U_CLAMP             UINT32_C(0x00000002) //!< Wrap U mode: Clamp
#define CT_RENDER_SAMPLER_U_BORDER            UINT32_C(0x00000003) //!< Wrap U mode: Border
#define CT_RENDER_SAMPLER_U_SHIFT             0                    //!<
#define CT_RENDER_SAMPLER_U_MASK              UINT32_C(0x00000003) //!<
#define CT_RENDER_SAMPLER_V_MIRROR            UINT32_C(0x00000004) //!< Wrap V mode: Mirror
#define CT_RENDER_SAMPLER_V_CLAMP             UINT32_C(0x00000008) //!< Wrap V mode: Clamp
#define CT_RENDER_SAMPLER_V_BORDER            UINT32_C(0x0000000c) //!< Wrap V mode: Border
#define CT_RENDER_SAMPLER_V_SHIFT             2                    //!<
#define CT_RENDER_SAMPLER_V_MASK              UINT32_C(0x0000000c) //!<
#define CT_RENDER_SAMPLER_W_MIRROR            UINT32_C(0x00000010) //!< Wrap W mode: Mirror
#define CT_RENDER_SAMPLER_W_CLAMP             UINT32_C(0x00000020) //!< Wrap W mode: Clamp
#define CT_RENDER_SAMPLER_W_BORDER            UINT32_C(0x00000030) //!< Wrap W mode: Border
#define CT_RENDER_SAMPLER_W_SHIFT             4                    //!<
#define CT_RENDER_SAMPLER_W_MASK              UINT32_C(0x00000030) //!<
#define CT_RENDER_SAMPLER_MIN_POINT           UINT32_C(0x00000040) //!< Min sampling mode: Point
#define CT_RENDER_SAMPLER_MIN_ANISOTROPIC     UINT32_C(0x00000080) //!< Min sampling mode: Anisotropic
#define CT_RENDER_SAMPLER_MIN_SHIFT           6                    //!<
#define CT_RENDER_SAMPLER_MIN_MASK            UINT32_C(0x000000c0) //!<
#define CT_RENDER_SAMPLER_MAG_POINT           UINT32_C(0x00000100) //!< Mag sampling mode: Point
#define CT_RENDER_SAMPLER_MAG_ANISOTROPIC     UINT32_C(0x00000200) //!< Mag sampling mode: Anisotropic
#define CT_RENDER_SAMPLER_MAG_SHIFT           8                    //!<
#define CT_RENDER_SAMPLER_MAG_MASK            UINT32_C(0x00000300) //!<
#define CT_RENDER_SAMPLER_MIP_POINT           UINT32_C(0x00000400) //!< Mip sampling mode: Point
#define CT_RENDER_SAMPLER_MIP_SHIFT           10                   //!<
#define CT_RENDER_SAMPLER_MIP_MASK            UINT32_C(0x00000400) //!<
#define CT_RENDER_SAMPLER_COMPARE_LESS        UINT32_C(0x00010000) //!< Compare when sampling depth texture: less.
#define CT_RENDER_SAMPLER_COMPARE_LEQUAL      UINT32_C(0x00020000) //!< Compare when sampling depth texture: less or equal.
#define CT_RENDER_SAMPLER_COMPARE_EQUAL       UINT32_C(0x00030000) //!< Compare when sampling depth texture: equal.
#define CT_RENDER_SAMPLER_COMPARE_GEQUAL      UINT32_C(0x00040000) //!< Compare when sampling depth texture: greater or equal.
#define CT_RENDER_SAMPLER_COMPARE_GREATER     UINT32_C(0x00050000) //!< Compare when sampling depth texture: greater.
#define CT_RENDER_SAMPLER_COMPARE_NOTEQUAL    UINT32_C(0x00060000) //!< Compare when sampling depth texture: not equal.
#define CT_RENDER_SAMPLER_COMPARE_NEVER       UINT32_C(0x00070000) //!< Compare when sampling depth texture: never.
#define CT_RENDER_SAMPLER_COMPARE_ALWAYS      UINT32_C(0x00080000) //!< Compare when sampling depth texture: always.
#define CT_RENDER_SAMPLER_COMPARE_SHIFT       16                   //!<
#define CT_RENDER_SAMPLER_COMPARE_MASK        UINT32_C(0x000f0000) //!<
#define CT_RENDER_SAMPLER_SAMPLE_STENCIL      UINT32_C(0x00100000) //!< Sample stencil instead of depth.
#define CT_RENDER_SAMPLER_BORDER_COLOR_SHIFT  24                   //!<
#define CT_RENDER_SAMPLER_BORDER_COLOR_MASK   UINT32_C(0x0f000000) //!<
#define CT_RENDER_SAMPLER_RESERVED_SHIFT      28                   //!<
#define CT_RENDER_SAMPLER_RESERVED_MASK       UINT32_C(0xf0000000) //!<

#define CT_RENDER_SAMPLER_BORDER_COLOR(_index) ( (_index << CT_RENDER_SAMPLER_BORDER_COLOR_SHIFT) & CT_RENDER_SAMPLER_BORDER_COLOR_MASK)

#define CT_RENDER_SAMPLER_BITS_MASK (0 \
            | CT_RENDER_SAMPLER_U_MASK         \
            | CT_RENDER_SAMPLER_V_MASK         \
            | CT_RENDER_SAMPLER_W_MASK         \
            | CT_RENDER_SAMPLER_MIN_MASK       \
            | CT_RENDER_SAMPLER_MAG_MASK       \
            | CT_RENDER_SAMPLER_MIP_MASK       \
            | CT_RENDER_SAMPLER_COMPARE_MASK   \
            )

///
#define CT_RENDER_RESET_NONE                  UINT32_C(0x00000000) //!< No reset flags.
#define CT_RENDER_RESET_FULLSCREEN            UINT32_C(0x00000001) //!< Not supported yet.
#define CT_RENDER_RESET_FULLSCREEN_SHIFT      0                    //!< Fullscreen bit shift.
#define CT_RENDER_RESET_FULLSCREEN_MASK       UINT32_C(0x00000001) //!< Fullscreen bit mask.
#define CT_RENDER_RESET_MSAA_X2               UINT32_C(0x00000010) //!< Enable 2x MSAA.
#define CT_RENDER_RESET_MSAA_X4               UINT32_C(0x00000020) //!< Enable 4x MSAA.
#define CT_RENDER_RESET_MSAA_X8               UINT32_C(0x00000030) //!< Enable 8x MSAA.
#define CT_RENDER_RESET_MSAA_X16              UINT32_C(0x00000040) //!< Enable 16x MSAA.
#define CT_RENDER_RESET_MSAA_SHIFT            4                    //!< MSAA mode bit shift.
#define CT_RENDER_RESET_MSAA_MASK             UINT32_C(0x00000070) //!< MSAA mode bit mask.
#define CT_RENDER_RESET_VSYNC                 UINT32_C(0x00000080) //!< Enable V-Sync.
#define CT_RENDER_RESET_MAXANISOTROPY         UINT32_C(0x00000100) //!< Turn on/off max anisotropy.
#define CT_RENDER_RESET_CAPTURE               UINT32_C(0x00000200) //!< Begin screen capture.
#define CT_RENDER_RESET_FLUSH_AFTER_RENDER    UINT32_C(0x00002000) //!< Flush rendering after submitting to GPU.
#define CT_RENDER_RESET_FLIP_AFTER_RENDER     UINT32_C(0x00004000) //!< This flag  specifies where flip occurs. Default behavior is that flip occurs before rendering new frame. This flag only has effect when `CT_RENDER_CONFIG_MULTITHREADED=0`.
#define CT_RENDER_RESET_SRGB_BACKBUFFER       UINT32_C(0x00008000) //!< Enable sRGB backbuffer.
#define CT_RENDER_RESET_HDR10                 UINT32_C(0x00010000) //!< Enable HDR10 rendering.
#define CT_RENDER_RESET_HIDPI                 UINT32_C(0x00020000) //!< Enable HiDPI rendering.
#define CT_RENDER_RESET_DEPTH_CLAMP           UINT32_C(0x00040000) //!< Enable depth clamp.
#define CT_RENDER_RESET_SUSPEND               UINT32_C(0x00080000) //!< Suspend rendering.

#define CT_RENDER_RESET_RESERVED_SHIFT        31                   //!< Internal bits shift.
#define CT_RENDER_RESET_RESERVED_MASK         UINT32_C(0x80000000) //!< Internal bits mask.

///
#define CT_RENDER_CAPS_ALPHA_TO_COVERAGE      UINT64_C(0x0000000000000001) //!< Alpha to coverage is supported.
#define CT_RENDER_CAPS_BLEND_INDEPENDENT      UINT64_C(0x0000000000000002) //!< Blend independent is supported.
#define CT_RENDER_CAPS_COMPUTE                UINT64_C(0x0000000000000004) //!< Compute shaders are supported.
#define CT_RENDER_CAPS_CONSERVATIVE_RASTER    UINT64_C(0x0000000000000008) //!< Conservative rasterization is supported.
#define CT_RENDER_CAPS_DRAW_INDIRECT          UINT64_C(0x0000000000000010) //!< Draw indirect is supported.
#define CT_RENDER_CAPS_FRAGMENT_DEPTH         UINT64_C(0x0000000000000020) //!< Fragment depth is accessible in fragment shader.
#define CT_RENDER_CAPS_FRAGMENT_ORDERING      UINT64_C(0x0000000000000040) //!< Fragment ordering is available in fragment shader.
#define CT_RENDER_CAPS_GRAPHICS_DEBUGGER      UINT64_C(0x0000000000000080) //!< Graphics debugger is present.
#define CT_RENDER_CAPS_HDR10                  UINT64_C(0x0000000000000100) //!< HDR10 rendering is supported.
#define CT_RENDER_CAPS_HIDPI                  UINT64_C(0x0000000000000400) //!< HiDPI rendering is supported.
#define CT_RENDER_CAPS_INDEX32                UINT64_C(0x0000000000000800) //!< 32-bit indices are supported.
#define CT_RENDER_CAPS_INSTANCING             UINT64_C(0x0000000000001000) //!< Instancing is supported.
#define CT_RENDER_CAPS_OCCLUSION_QUERY        UINT64_C(0x0000000000002000) //!< Occlusion query is supported.
#define CT_RENDER_CAPS_RENDERER_MULTITHREADED UINT64_C(0x0000000000004000) //!< Renderer is on separate thread.
#define CT_RENDER_CAPS_SWAP_CHAIN             UINT64_C(0x0000000000008000) //!< Multiple windows are supported.
#define CT_RENDER_CAPS_TEXTURE_2D_ARRAY       UINT64_C(0x0000000000010000) //!< 2D texture array is supported.
#define CT_RENDER_CAPS_TEXTURE_3D             UINT64_C(0x0000000000020000) //!< 3D textures are supported.
#define CT_RENDER_CAPS_TEXTURE_BLIT           UINT64_C(0x00000000000c0000) //!< Texture blit is supported.
#define CT_RENDER_CAPS_TEXTURE_COMPARE_ALL    UINT64_C(0x0000000000080000) //!< All texture compare modes are supported.
#define CT_RENDER_CAPS_TEXTURE_COMPARE_LEQUAL UINT64_C(0x0000000000100000) //!< Texture compare less equal mode is supported.
#define CT_RENDER_CAPS_TEXTURE_CUBE_ARRAY     UINT64_C(0x0000000000200000) //!< Cubemap texture array is supported.
#define CT_RENDER_CAPS_TEXTURE_DIRECT_ACCESS  UINT64_C(0x0000000000400000) //!< CPU direct access to GPU texture memory.
#define CT_RENDER_CAPS_TEXTURE_READ_BACK      UINT64_C(0x0000000000800000) //!< Read-back texture is supported.
#define CT_RENDER_CAPS_VERTEX_ATTRIB_HALF     UINT64_C(0x0000000000800000) //!< Vertex attribute half-float is supported.
#define CT_RENDER_CAPS_VERTEX_ATTRIB_UINT10   UINT64_C(0x0000000001000000) //!< Vertex attribute 10_10_10_2 is supported.
#define CT_RENDER_CAPS_VERTEX_ID              UINT64_C(0x0000000002000000) //!< Rendering with VertexID only is supported.

///
#define CT_RENDER_CAPS_FORMAT_TEXTURE_NONE             UINT16_C(0x0000) //!< Texture format is not supported.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_2D               UINT16_C(0x0001) //!< Texture format is supported.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_2D_SRGB          UINT16_C(0x0002) //!< Texture as sRGB format is supported.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_2D_EMULATED      UINT16_C(0x0004) //!< Texture format is emulated.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_3D               UINT16_C(0x0008) //!< Texture format is supported.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_3D_SRGB          UINT16_C(0x0010) //!< Texture as sRGB format is supported.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_3D_EMULATED      UINT16_C(0x0020) //!< Texture format is emulated.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_CUBE             UINT16_C(0x0040) //!< Texture format is supported.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_CUBE_SRGB        UINT16_C(0x0080) //!< Texture as sRGB format is supported.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_CUBE_EMULATED    UINT16_C(0x0100) //!< Texture format is emulated.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_VERTEX           UINT16_C(0x0200) //!< Texture format can be used from vertex shader.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_IMAGE            UINT16_C(0x0400) //!< Texture format can be used as image from compute shader.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_FRAMEBUFFER      UINT16_C(0x0800) //!< Texture format can be used as frame buffer.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_FRAMEBUFFER_MSAA UINT16_C(0x1000) //!< Texture format can be used as MSAA frame buffer.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_MSAA             UINT16_C(0x2000) //!< Texture can be sampled as MSAA.
#define CT_RENDER_CAPS_FORMAT_TEXTURE_MIP_AUTOGEN      UINT16_C(0x4000) //!< Texture format supports auto-generated mips.

///
#define CT_RENDER_VIEW_NONE   UINT8_C(0x00) //!<
#define CT_RENDER_VIEW_STEREO UINT8_C(0x01) //!< View will be rendered in stereo mode.

///
#define CT_RENDER_SUBMIT_EYE_LEFT       UINT8_C(0x01) //!< Submit to left eye.
#define CT_RENDER_SUBMIT_EYE_RIGHT      UINT8_C(0x02) //!< Submit to right eye.
#define CT_RENDER_SUBMIT_EYE_MASK       UINT8_C(0x03) //!<
#define CT_RENDER_SUBMIT_EYE_FIRST      CT_RENDER_SUBMIT_EYE_LEFT

#define CT_RENDER_SUBMIT_RESERVED_SHIFT 7             //!< Internal bits shift.
#define CT_RENDER_SUBMIT_RESERVED_MASK  UINT8_C(0x80) //!< Internal bits mask.

///
#define CT_RENDER_PCI_ID_NONE                UINT16_C(0x0000) //!< Autoselect adapter.
#define CT_RENDER_PCI_ID_SOFTWARE_RASTERIZER UINT16_C(0x0001) //!< Software rasterizer.
#define CT_RENDER_PCI_ID_AMD                 UINT16_C(0x1002) //!< AMD adapter.
#define CT_RENDER_PCI_ID_INTEL               UINT16_C(0x8086) //!< Intel adapter.
#define CT_RENDER_PCI_ID_NVIDIA              UINT16_C(0x10de) //!< nVidia adapter.

///
#define CT_RENDER_CUBE_MAP_POSITIVE_X UINT8_C(0x00) //!< Cubemap +x.
#define CT_RENDER_CUBE_MAP_NEGATIVE_X UINT8_C(0x01) //!< Cubemap -x.
#define CT_RENDER_CUBE_MAP_POSITIVE_Y UINT8_C(0x02) //!< Cubemap +y.
#define CT_RENDER_CUBE_MAP_NEGATIVE_Y UINT8_C(0x03) //!< Cubemap -y.
#define CT_RENDER_CUBE_MAP_POSITIVE_Z UINT8_C(0x04) //!< Cubemap +z.
#define CT_RENDER_CUBE_MAP_NEGATIVE_Z UINT8_C(0x05) //!< Cubemap -z.


///

typedef enum ct_render_renderer_type {
    CT_RENDER_RENDERER_TYPE_NOOP,
    CT_RENDER_RENDERER_TYPE_DIRECT3D9,
    CT_RENDER_RENDERER_TYPE_DIRECT3D11,
    CT_RENDER_RENDERER_TYPE_DIRECT3D12,
    CT_RENDER_RENDERER_TYPE_GNM,
    CT_RENDER_RENDERER_TYPE_METAL,
    CT_RENDER_RENDERER_TYPE_OPENGLES,
    CT_RENDER_RENDERER_TYPE_OPENGL,
    CT_RENDER_RENDERER_TYPE_VULKAN,

    CT_RENDER_RENDERER_TYPE_COUNT

} ct_render_renderer_type_t;

typedef enum ct_render_access {
    CT_RENDER_ACCESS_READ,
    CT_RENDER_ACCESS_WRITE,
    CT_RENDER_ACCESS_READWRITE,

    CT_RENDER_ACCESS_COUNT

} ct_render_access_t;

typedef enum ct_render_attrib {
    CT_RENDER_ATTRIB_POSITION,
    CT_RENDER_ATTRIB_NORMAL,
    CT_RENDER_ATTRIB_TANGENT,
    CT_RENDER_ATTRIB_BITANGENT,
    CT_RENDER_ATTRIB_COLOR0,
    CT_RENDER_ATTRIB_COLOR1,
    CT_RENDER_ATTRIB_COLOR2,
    CT_RENDER_ATTRIB_COLOR3,
    CT_RENDER_ATTRIB_INDICES,
    CT_RENDER_ATTRIB_WEIGHT,
    CT_RENDER_ATTRIB_TEXCOORD0,
    CT_RENDER_ATTRIB_TEXCOORD1,
    CT_RENDER_ATTRIB_TEXCOORD2,
    CT_RENDER_ATTRIB_TEXCOORD3,
    CT_RENDER_ATTRIB_TEXCOORD4,
    CT_RENDER_ATTRIB_TEXCOORD5,
    CT_RENDER_ATTRIB_TEXCOORD6,
    CT_RENDER_ATTRIB_TEXCOORD7,

    CT_RENDER_ATTRIB_COUNT

} ct_render_attrib_t;

typedef enum ct_render_attrib_type {
    CT_RENDER_ATTRIB_TYPE_UINT8,
    CT_RENDER_ATTRIB_TYPE_UINT10,
    CT_RENDER_ATTRIB_TYPE_INT16,
    CT_RENDER_ATTRIB_TYPE_HALF,
    CT_RENDER_ATTRIB_TYPE_FLOAT,

    CT_RENDER_ATTRIB_TYPE_COUNT

} ct_render_attrib_type_t;

typedef enum ct_render_texture_format {
    CT_RENDER_TEXTURE_FORMAT_BC1,
    CT_RENDER_TEXTURE_FORMAT_BC2,
    CT_RENDER_TEXTURE_FORMAT_BC3,
    CT_RENDER_TEXTURE_FORMAT_BC4,
    CT_RENDER_TEXTURE_FORMAT_BC5,
    CT_RENDER_TEXTURE_FORMAT_BC6H,
    CT_RENDER_TEXTURE_FORMAT_BC7,
    CT_RENDER_TEXTURE_FORMAT_ETC1,
    CT_RENDER_TEXTURE_FORMAT_ETC2,
    CT_RENDER_TEXTURE_FORMAT_ETC2A,
    CT_RENDER_TEXTURE_FORMAT_ETC2A1,
    CT_RENDER_TEXTURE_FORMAT_PTC12,
    CT_RENDER_TEXTURE_FORMAT_PTC14,
    CT_RENDER_TEXTURE_FORMAT_PTC12A,
    CT_RENDER_TEXTURE_FORMAT_PTC14A,
    CT_RENDER_TEXTURE_FORMAT_PTC22,
    CT_RENDER_TEXTURE_FORMAT_PTC24,
    CT_RENDER_TEXTURE_FORMAT_ATC,
    CT_RENDER_TEXTURE_FORMAT_ATCE,
    CT_RENDER_TEXTURE_FORMAT_ATCI,
    CT_RENDER_TEXTURE_FORMAT_ASTC4x4,
    CT_RENDER_TEXTURE_FORMAT_ASTC5x5,
    CT_RENDER_TEXTURE_FORMAT_ASTC6x6,
    CT_RENDER_TEXTURE_FORMAT_ASTC8x5,
    CT_RENDER_TEXTURE_FORMAT_ASTC8x6,
    CT_RENDER_TEXTURE_FORMAT_ASTC10x5,

    CT_RENDER_TEXTURE_FORMAT_UNKNOWN,

    CT_RENDER_TEXTURE_FORMAT_R1,
    CT_RENDER_TEXTURE_FORMAT_A8,
    CT_RENDER_TEXTURE_FORMAT_R8,
    CT_RENDER_TEXTURE_FORMAT_R8I,
    CT_RENDER_TEXTURE_FORMAT_R8U,
    CT_RENDER_TEXTURE_FORMAT_R8S,
    CT_RENDER_TEXTURE_FORMAT_R16,
    CT_RENDER_TEXTURE_FORMAT_R16I,
    CT_RENDER_TEXTURE_FORMAT_R16U,
    CT_RENDER_TEXTURE_FORMAT_R16F,
    CT_RENDER_TEXTURE_FORMAT_R16S,
    CT_RENDER_TEXTURE_FORMAT_R32I,
    CT_RENDER_TEXTURE_FORMAT_R32U,
    CT_RENDER_TEXTURE_FORMAT_R32F,
    CT_RENDER_TEXTURE_FORMAT_RG8,
    CT_RENDER_TEXTURE_FORMAT_RG8I,
    CT_RENDER_TEXTURE_FORMAT_RG8U,
    CT_RENDER_TEXTURE_FORMAT_RG8S,
    CT_RENDER_TEXTURE_FORMAT_RG16,
    CT_RENDER_TEXTURE_FORMAT_RG16I,
    CT_RENDER_TEXTURE_FORMAT_RG16U,
    CT_RENDER_TEXTURE_FORMAT_RG16F,
    CT_RENDER_TEXTURE_FORMAT_RG16S,
    CT_RENDER_TEXTURE_FORMAT_RG32I,
    CT_RENDER_TEXTURE_FORMAT_RG32U,
    CT_RENDER_TEXTURE_FORMAT_RG32F,
    CT_RENDER_TEXTURE_FORMAT_RGB8,
    CT_RENDER_TEXTURE_FORMAT_RGB8I,
    CT_RENDER_TEXTURE_FORMAT_RGB8U,
    CT_RENDER_TEXTURE_FORMAT_RGB8S,
    CT_RENDER_TEXTURE_FORMAT_RGB9E5F,
    CT_RENDER_TEXTURE_FORMAT_BGRA8,
    CT_RENDER_TEXTURE_FORMAT_RGBA8,
    CT_RENDER_TEXTURE_FORMAT_RGBA8I,
    CT_RENDER_TEXTURE_FORMAT_RGBA8U,
    CT_RENDER_TEXTURE_FORMAT_RGBA8S,
    CT_RENDER_TEXTURE_FORMAT_RGBA16,
    CT_RENDER_TEXTURE_FORMAT_RGBA16I,
    CT_RENDER_TEXTURE_FORMAT_RGBA16U,
    CT_RENDER_TEXTURE_FORMAT_RGBA16F,
    CT_RENDER_TEXTURE_FORMAT_RGBA16S,
    CT_RENDER_TEXTURE_FORMAT_RGBA32I,
    CT_RENDER_TEXTURE_FORMAT_RGBA32U,
    CT_RENDER_TEXTURE_FORMAT_RGBA32F,
    CT_RENDER_TEXTURE_FORMAT_R5G6B5,
    CT_RENDER_TEXTURE_FORMAT_RGBA4,
    CT_RENDER_TEXTURE_FORMAT_RGB5A1,
    CT_RENDER_TEXTURE_FORMAT_RGB10A2,
    CT_RENDER_TEXTURE_FORMAT_RG11B10F,

    CT_RENDER_TEXTURE_FORMAT_UNKNOWN_DEPTH,

    CT_RENDER_TEXTURE_FORMAT_D16,
    CT_RENDER_TEXTURE_FORMAT_D24,
    CT_RENDER_TEXTURE_FORMAT_D24S8,
    CT_RENDER_TEXTURE_FORMAT_D32,
    CT_RENDER_TEXTURE_FORMAT_D16F,
    CT_RENDER_TEXTURE_FORMAT_D24F,
    CT_RENDER_TEXTURE_FORMAT_D32F,
    CT_RENDER_TEXTURE_FORMAT_D0S8,

    CT_RENDER_TEXTURE_FORMAT_COUNT

} ct_render_texture_format_t;

typedef enum ct_render_uniform_type {
    CT_RENDER_UNIFORM_TYPE_INT1,
    CT_RENDER_UNIFORM_TYPE_END,

    CT_RENDER_UNIFORM_TYPE_VEC4,
    CT_RENDER_UNIFORM_TYPE_MAT3,
    CT_RENDER_UNIFORM_TYPE_MAT4,

    CT_RENDER_UNIFORM_TYPE_COUNT

} ct_render_uniform_type_t;

typedef enum ct_render_backbuffer_ratio {
    CT_RENDER_BACKBUFFER_RATIO_EQUAL,
    CT_RENDER_BACKBUFFER_RATIO_HALF,
    CT_RENDER_BACKBUFFER_RATIO_QUARTER,
    CT_RENDER_BACKBUFFER_RATIO_EIGHTH,
    CT_RENDER_BACKBUFFER_RATIO_SIXTEENTH,
    CT_RENDER_BACKBUFFER_RATIO_DOUBLE,

    CT_RENDER_BACKBUFFER_RATIO_COUNT

} ct_render_backbuffer_ratio_t;

typedef enum ct_render_occlusion_query_result {
    CT_RENDER_OCCLUSION_QUERY_RESULT_INVISIBLE,
    CT_RENDER_OCCLUSION_QUERY_RESULT_VISIBLE,
    CT_RENDER_OCCLUSION_QUERY_RESULT_NORESULT,

    CT_RENDER_OCCLUSION_QUERY_RESULT_COUNT

} ct_render_occlusion_query_result_t;

typedef enum ct_render_topology {
    CT_RENDER_TOPOLOGY_TRI_LIST,
    CT_RENDER_TOPOLOGY_TRI_STRIP,
    CT_RENDER_TOPOLOGY_LINE_LIST,
    CT_RENDER_TOPOLOGY_LINE_STRIP,
    CT_RENDER_TOPOLOGY_POINT_LIST,

    CT_RENDER_TOPOLOGY_COUNT

} ct_render_topology_t;

typedef enum ct_render_topology_convert {
    CT_RENDER_TOPOLOGY_CONVERT_TRI_LIST_FLIP_WINDING,
    CT_RENDER_TOPOLOGY_CONVERT_TRI_STRIP_FLIP_WINDING,
    CT_RENDER_TOPOLOGY_CONVERT_TRI_LIST_TO_LINE_LIST,
    CT_RENDER_TOPOLOGY_CONVERT_TRI_STRIP_TO_TRI_LIST,
    CT_RENDER_TOPOLOGY_CONVERT_LINE_STRIP_TO_LINE_LIST,

    CT_RENDER_TOPOLOGY_CONVERT_COUNT

} ct_render_topology_convert_t;

typedef enum ct_render_topology_sort {
    CT_RENDER_TOPOLOGY_SORT_DIRECTION_FRONT_TO_BACK_MIN,
    CT_RENDER_TOPOLOGY_SORT_DIRECTION_FRONT_TO_BACK_AVG,
    CT_RENDER_TOPOLOGY_SORT_DIRECTION_FRONT_TO_BACK_MAX,
    CT_RENDER_TOPOLOGY_SORT_DIRECTION_BACK_TO_FRONT_MIN,
    CT_RENDER_TOPOLOGY_SORT_DIRECTION_BACK_TO_FRONT_AVG,
    CT_RENDER_TOPOLOGY_SORT_DIRECTION_BACK_TO_FRONT_MAX,
    CT_RENDER_TOPOLOGY_SORT_DISTANCE_FRONT_TO_BACK_MIN,
    CT_RENDER_TOPOLOGY_SORT_DISTANCE_FRONT_TO_BACK_AVG,
    CT_RENDER_TOPOLOGY_SORT_DISTANCE_FRONT_TO_BACK_MAX,
    CT_RENDER_TOPOLOGY_SORT_DISTANCE_BACK_TO_FRONT_MIN,
    CT_RENDER_TOPOLOGY_SORT_DISTANCE_BACK_TO_FRONT_AVG,
    CT_RENDER_TOPOLOGY_SORT_DISTANCE_BACK_TO_FRONT_MAX,

    CT_RENDER_TOPOLOGY_SORT_COUNT

} ct_render_topology_sort_t;

typedef enum ct_render_view_mode {
    CT_RENDER_VIEW_MODE_DEFAULT,
    CT_RENDER_VIEW_MODE_SEQUENTIAL,
    CT_RENDER_VIEW_MODE_DEPTH_ASCENDING,
    CT_RENDER_VIEW_MODE_DEPTH_DESCENDING,

    CT_RENDER_VIEW_MODE_CCOUNT

} ct_render_view_mode_t;

#define CT_RENDER_HANDLE_T(_name) \
    typedef struct _name{ uint16_t idx; } _name##_t

CT_RENDER_HANDLE_T(ct_render_dynamic_index_buffer_handle);
CT_RENDER_HANDLE_T(ct_render_dynamic_vertex_buffer_handle);
CT_RENDER_HANDLE_T(ct_render_frame_buffer_handle);
CT_RENDER_HANDLE_T(ct_render_index_buffer_handle);
CT_RENDER_HANDLE_T(ct_render_indirect_buffer_handle);
CT_RENDER_HANDLE_T(ct_render_occlusion_query_handle);
CT_RENDER_HANDLE_T(ct_render_program_handle);
CT_RENDER_HANDLE_T(ct_render_shader_handle);
CT_RENDER_HANDLE_T(ct_render_texture_handle);
CT_RENDER_HANDLE_T(ct_render_uniform_handle);
CT_RENDER_HANDLE_T(ct_render_vertex_buffer_handle);
CT_RENDER_HANDLE_T(ct_render_vertex_decl_handle);

#undef CT_RENDER_HANDLE_T


typedef void (*ct_render_release_fn_t)(void *_ptr,
                                       void *_userData);


typedef struct ct_render_memory {
    uint8_t *data;
    uint32_t size;

} ct_render_memory_t;


typedef struct ct_render_transform {
    float *data;
    uint16_t num;

} ct_render_transform_t;


typedef uint16_t ct_render_view_id_t;


typedef struct ct_render_view_stats {
    char name[256];
    ct_render_view_id_t view;
    int64_t cpuTimeElapsed;
    int64_t gpuTimeElapsed;

} ct_render_view_stats_t;

typedef struct ct_render_encoder_stats {
    int64_t cpuTimeBegin;
    int64_t cpuTimeEnd;

} ct_render_encoder_stats_t;


typedef struct ct_render_stats {
    int64_t cpuTimeFrame;
    int64_t cpuTimeBegin;
    int64_t cpuTimeEnd;
    int64_t cpuTimerFreq;

    int64_t gpuTimeBegin;
    int64_t gpuTimeEnd;
    int64_t gpuTimerFreq;

    int64_t waitRender;
    int64_t waitSubmit;

    uint32_t numDraw;
    uint32_t numCompute;
    uint32_t maxGpuLatency;

    uint16_t numDynamicIndexBuffers;
    uint16_t numDynamicVertexBuffers;
    uint16_t numFrameBuffers;
    uint16_t numIndexBuffers;
    uint16_t numOcclusionQueries;
    uint16_t numPrograms;
    uint16_t numShaders;
    uint16_t numTextures;
    uint16_t numUniforms;
    uint16_t numVertexBuffers;
    uint16_t numVertexDecls;

    int64_t textureMemoryUsed;
    int64_t rtMemoryUsed;
    int32_t transientVbUsed;
    int32_t transientIbUsed;

    uint32_t numPrims[CT_RENDER_TOPOLOGY_COUNT];

    int64_t gpuMemoryMax;
    int64_t gpuMemoryUsed;

    uint16_t width;
    uint16_t height;
    uint16_t textWidth;
    uint16_t textHeight;

    uint16_t numViews;
    ct_render_view_stats_t *viewStats;

    uint8_t numEncoders;
    ct_render_encoder_stats_t *encoderStats;

} ct_render_stats_t;


typedef struct ct_render_vertex_decl {
    uint32_t hash;
    uint16_t stride;
    uint16_t offset[CT_RENDER_ATTRIB_COUNT];
    uint16_t attributes[CT_RENDER_ATTRIB_COUNT];

} ct_render_vertex_decl_t;


typedef struct ct_render_transient_index_buffer {
    uint8_t *data;
    uint32_t size;
    ct_render_index_buffer_handle_t handle;
    uint32_t startIndex;

} ct_render_transient_index_buffer_t;


typedef struct ct_render_transient_vertex_buffer {
    uint8_t *data;
    uint32_t size;
    uint32_t startVertex;
    uint16_t stride;
    ct_render_vertex_buffer_handle_t handle;
    ct_render_vertex_decl_handle_t decl;

} ct_render_transient_vertex_buffer_t;


typedef struct ct_render_instance_data_buffer {
    uint8_t *data;
    uint32_t size;
    uint32_t offset;
    uint32_t num;
    uint16_t stride;
    ct_render_vertex_buffer_handle_t handle;

} ct_render_instance_data_buffer_t;


typedef struct ct_render_texture_info {
    ct_render_texture_format_t format;
    uint32_t storageSize;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t numLayers;
    uint8_t numMips;
    uint8_t bitsPerPixel;
    bool cubeMap;

} ct_render_texture_info_t;


typedef struct ct_render_uniform_info {
    char name[256];
    ct_render_uniform_type_t type;
    uint16_t num;

} ct_render_uniform_info_t;


typedef struct ct_render_attachment {
    ct_render_texture_handle_t handle;
    uint16_t mip;
    uint16_t layer;

} ct_render_attachment_t;


typedef struct ct_render_caps_gpu {
    uint16_t vendorId;
    uint16_t deviceId;

} ct_render_caps_gpu_t;

typedef struct ct_render_caps_limits {
    uint32_t maxDrawCalls;
    uint32_t maxBlits;
    uint32_t maxTextureSize;
    uint32_t maxTextureLayers;
    uint32_t maxViews;
    uint32_t maxFrameBuffers;
    uint32_t maxFBAttachments;
    uint32_t maxPrograms;
    uint32_t maxShaders;
    uint32_t maxTextures;
    uint32_t maxTextureSamplers;
    uint32_t maxVertexDecls;
    uint32_t maxVertexStreams;
    uint32_t maxIndexBuffers;
    uint32_t maxVertexBuffers;
    uint32_t maxDynamicIndexBuffers;
    uint32_t maxDynamicVertexBuffers;
    uint32_t maxUniforms;
    uint32_t maxOcclusionQueries;
    uint32_t maxEncoders;
    uint32_t transientVbSize;
    uint32_t transientIbSize;

} ct_render_caps_limits_t;


typedef struct ct_render_caps {
    ct_render_renderer_type_t rendererType;

    uint64_t supported;

    uint16_t vendorId;
    uint16_t deviceId;
    bool homogeneousDepth;
    bool originBottomLeft;
    uint8_t numGPUs;

    ct_render_caps_gpu_t gpu[4];
    ct_render_caps_limits_t limits;

    uint16_t formats[CT_RENDER_TEXTURE_FORMAT_COUNT];

} ct_render_caps_t;

struct ct_render_encoder;

struct ct_gfx_a0 {
    ///
    void (*vertex_decl_begin)(ct_render_vertex_decl_t *_decl,
                              ct_render_renderer_type_t renderer);

    void (*vertex_decl_add)(ct_render_vertex_decl_t *_decl,
                            ct_render_attrib_t attrib,
                            uint8_t num,
                            ct_render_attrib_type_t type,
                            bool normalized,
                            bool asInt);

    void (*vertex_decl_skip)(ct_render_vertex_decl_t *_decl,
                             uint8_t num);

    void (*vertex_decl_end)(ct_render_vertex_decl_t *_decl);

    void (*vertex_pack)(const float input[4],
                        bool inputNormalized,
                        ct_render_attrib_t attr,
                        const ct_render_vertex_decl_t *_decl,
                        void *_data,
                        uint32_t index);

    void (*vertex_unpack)(float output[4],
                          ct_render_attrib_t attr,
                          const ct_render_vertex_decl_t *_decl,
                          const void *_data,
                          uint32_t index);

    void (*vertex_convert)(const ct_render_vertex_decl_t *_destDecl,
                           void *_destData,
                           const ct_render_vertex_decl_t *_srcDecl,
                           const void *_srcData,
                           uint32_t num);

    uint16_t (*weld_vertices)(uint16_t *_output,
                              const ct_render_vertex_decl_t *_decl,
                              const void *_data,
                              uint16_t num,
                              float epsilon);

    uint32_t (*topology_convert)(ct_render_topology_convert_t conversion,
                                 void *_dst,
                                 uint32_t dstSize,
                                 const void *_indices,
                                 uint32_t numIndices,
                                 bool index32);

    void (*topology_sort_tri_list)(ct_render_topology_sort_t sort,
                                   void *_dst,
                                   uint32_t dstSize,
                                   const float dir[3],
                                   const float pos[3],
                                   const void *_vertices,
                                   uint32_t stride,
                                   const void *_indices,
                                   uint32_t numIndices,
                                   bool index32);

    uint8_t (*get_supported_renderers)(uint8_t max,
                                       ct_render_renderer_type_t *_enum);

    const char *(*get_renderer_name)(ct_render_renderer_type_t type);

    uint32_t (*frame)(bool capture);

    ct_render_renderer_type_t (*get_renderer_type)();

    const ct_render_caps_t *(*get_caps)();

    const ct_render_stats_t *(*get_stats)();

    const ct_render_memory_t *(*alloc)(uint32_t size);

    const ct_render_memory_t *(*copy)(const void *_data,
                                      uint32_t size);

    const ct_render_memory_t *(*make_ref)(const void *_data,
                                          uint32_t size);

    const ct_render_memory_t *(*make_ref_release)(const void *_data,
                                                  uint32_t size,
                                                  ct_render_release_fn_t releaseFn,
                                                  void *_userData);

    void (*dbg_text_clear)(uint8_t attr,
                           bool small);

    void (*dbg_text_printf)(uint16_t x,
                            uint16_t y,
                            uint8_t attr,
                            const char *_format,
                            ...);

    void (*dbg_text_vprintf)(uint16_t x,
                             uint16_t y,
                             uint8_t attr,
                             const char *_format,
                             va_list argList);

    void (*dbg_text_image)(uint16_t x,
                           uint16_t y,
                           uint16_t width,
                           uint16_t height,
                           const void *_data,
                           uint16_t pitch);

    ct_render_index_buffer_handle_t
    (*create_index_buffer)(const ct_render_memory_t *_mem,
                           uint16_t flags);

    void (*destroy_index_buffer)(ct_render_index_buffer_handle_t handle);

    ct_render_vertex_buffer_handle_t
    (*create_vertex_buffer)(const ct_render_memory_t *_mem,
                            const ct_render_vertex_decl_t *_decl,
                            uint16_t flags);

    void (*destroy_vertex_buffer)(ct_render_vertex_buffer_handle_t handle);

    ct_render_dynamic_index_buffer_handle_t
    (*create_dynamic_index_buffer)(uint32_t num,
                                   uint16_t flags);

    ct_render_dynamic_index_buffer_handle_t
    (*create_dynamic_index_buffer_mem)(const ct_render_memory_t *_mem,
                                       uint16_t flags);

    void
    (*update_dynamic_index_buffer)(ct_render_dynamic_index_buffer_handle_t handle,
                                   uint32_t startIndex,
                                   const ct_render_memory_t *_mem);

    void
    (*destroy_dynamic_index_buffer)(ct_render_dynamic_index_buffer_handle_t handle);

    ct_render_dynamic_vertex_buffer_handle_t
    (*create_dynamic_vertex_buffer)(uint32_t num,
                                    const ct_render_vertex_decl_t *_decl,
                                    uint16_t flags);

    ct_render_dynamic_vertex_buffer_handle_t
    (*create_dynamic_vertex_buffer_mem)(const ct_render_memory_t *_mem,
                                        const ct_render_vertex_decl_t *_decl,
                                        uint16_t flags);

    void
    (*update_dynamic_vertex_buffer)(ct_render_dynamic_vertex_buffer_handle_t handle,
                                    uint32_t startVertex,
                                    const ct_render_memory_t *_mem);

    void
    (*destroy_dynamic_vertex_buffer)(ct_render_dynamic_vertex_buffer_handle_t handle);

    uint32_t (*get_avail_transient_index_buffer)(uint32_t num);

    uint32_t (*get_avail_transient_vertex_buffer)(uint32_t num,
                                                  const ct_render_vertex_decl_t *_decl);

    uint32_t (*get_avail_instance_data_buffer)(uint32_t num,
                                               uint16_t stride);

    void
    (*alloc_transient_index_buffer)(ct_render_transient_index_buffer_t *_tib,
                                    uint32_t num);

    void
    (*alloc_transient_vertex_buffer)(ct_render_transient_vertex_buffer_t *_tvb,
                                     uint32_t num,
                                     const ct_render_vertex_decl_t *_decl);

    bool (*alloc_transient_buffers)(ct_render_transient_vertex_buffer_t *_tvb,
                                    const ct_render_vertex_decl_t *_decl,
                                    uint32_t numVertices,
                                    ct_render_transient_index_buffer_t *_tib,
                                    uint32_t numIndices);

    void (*alloc_instance_data_buffer)(ct_render_instance_data_buffer_t *_idb,
                                       uint32_t num,
                                       uint16_t stride);

    ct_render_indirect_buffer_handle_t (*create_indirect_buffer)(uint32_t num);

    void (*destroy_indirect_buffer)(ct_render_indirect_buffer_handle_t handle);

    ct_render_shader_handle_t (*create_shader)(const ct_render_memory_t *_mem);

    uint16_t (*get_shader_uniforms)(ct_render_shader_handle_t handle,
                                    ct_render_uniform_handle_t *_uniforms,
                                    uint16_t max);

    void (*set_shader_name)(ct_render_shader_handle_t handle,
                            const char *_name);

    void (*destroy_shader)(ct_render_shader_handle_t handle);

    ct_render_program_handle_t (*create_program)(ct_render_shader_handle_t vsh,
                                                 ct_render_shader_handle_t fsh,
                                                 bool destroyShaders);

    ct_render_program_handle_t
    (*create_compute_program)(ct_render_shader_handle_t csh,
                              bool destroyShaders);

    void (*destroy_program)(ct_render_program_handle_t handle);

    bool (*is_texture_valid)(uint16_t depth,
                             bool cubeMap,
                             uint16_t numLayers,
                             ct_render_texture_format_t format,
                             uint32_t flags);

    void (*calc_texture_size)(ct_render_texture_info_t *_info,
                              uint16_t width,
                              uint16_t height,
                              uint16_t depth,
                              bool cubeMap,
                              bool hasMips,
                              uint16_t numLayers,
                              ct_render_texture_format_t format);

    ct_render_texture_handle_t (*create_texture)(const ct_render_memory_t *_mem,
                                                 uint64_t flags,
                                                 uint8_t skip,
                                                 ct_render_texture_info_t *_info);

    ct_render_texture_handle_t (*create_texture_2d)(uint16_t width,
                                                    uint16_t height,
                                                    bool hasMips,
                                                    uint16_t numLayers,
                                                    ct_render_texture_format_t format,
                                                    uint64_t flags,
                                                    const ct_render_memory_t *_mem);

    ct_render_texture_handle_t
    (*create_texture_2d_scaled)(ct_render_backbuffer_ratio_t ratio,
                                bool hasMips,
                                uint16_t numLayers,
                                ct_render_texture_format_t format,
                                uint64_t flags);

    ct_render_texture_handle_t (*create_texture_3d)(uint16_t width,
                                                    uint16_t height,
                                                    uint16_t depth,
                                                    bool hasMips,
                                                    ct_render_texture_format_t format,
                                                    uint64_t flags,
                                                    const ct_render_memory_t *_mem);

    ct_render_texture_handle_t (*create_texture_cube)(uint16_t size,
                                                      bool hasMips,
                                                      uint16_t numLayers,
                                                      ct_render_texture_format_t format,
                                                      uint64_t flags,
                                                      const ct_render_memory_t *_mem);

    void (*update_texture_2d)(ct_render_texture_handle_t handle,
                              uint16_t layer,
                              uint8_t mip,
                              uint16_t x,
                              uint16_t y,
                              uint16_t width,
                              uint16_t height,
                              const ct_render_memory_t *_mem,
                              uint16_t pitch);

    void (*update_texture_3d)(ct_render_texture_handle_t handle,
                              uint8_t mip,
                              uint16_t x,
                              uint16_t y,
                              uint16_t z,
                              uint16_t width,
                              uint16_t height,
                              uint16_t depth,
                              const ct_render_memory_t *_mem);

    void (*update_texture_cube)(ct_render_texture_handle_t handle,
                                uint16_t layer,
                                uint8_t side,
                                uint8_t mip,
                                uint16_t x,
                                uint16_t y,
                                uint16_t width,
                                uint16_t height,
                                const ct_render_memory_t *_mem,
                                uint16_t pitch);

    uint32_t (*read_texture)(ct_render_texture_handle_t handle,
                             void *_data,
                             uint8_t mip);

    void (*set_texture_name)(ct_render_texture_handle_t handle,
                             const char *_name);

    void *(*get_direct_access_ptr)(ct_render_texture_handle_t handle);

    void (*destroy_texture)(ct_render_texture_handle_t handle);

    ct_render_frame_buffer_handle_t (*create_frame_buffer)(uint16_t width,
                                                           uint16_t height,
                                                           ct_render_texture_format_t format,
                                                           uint64_t textureFlags);

    ct_render_frame_buffer_handle_t
    (*create_frame_buffer_scaled)(ct_render_backbuffer_ratio_t ratio,
                                  ct_render_texture_format_t format,
                                  uint64_t textureFlags);

    ct_render_frame_buffer_handle_t
    (*create_frame_buffer_from_attachment)(uint8_t num,
                                           const ct_render_attachment_t *_attachment,
                                           bool destroyTextures);

    ct_render_frame_buffer_handle_t (*create_frame_buffer_from_nwh)(void *_nwh,
                                                                    uint16_t width,
                                                                    uint16_t height,
                                                                    ct_render_texture_format_t depthFormat);

    ct_render_frame_buffer_handle_t
    (*create_frame_buffer_from_handles)(uint8_t num,
                                        const ct_render_texture_handle_t *_handles,
                                        bool destroyTextures);

    ct_render_texture_handle_t
    (*get_texture)(ct_render_frame_buffer_handle_t handle,
                   uint8_t attachment);

    void (*destroy_frame_buffer)(ct_render_frame_buffer_handle_t handle);

    ct_render_uniform_handle_t (*create_uniform)(const char *_name,
                                                 ct_render_uniform_type_t type,
                                                 uint16_t num);

    void (*get_uniform_info)(ct_render_uniform_handle_t handle,
                             ct_render_uniform_info_t *_info);

    void (*destroy_uniform)(ct_render_uniform_handle_t handle);

    ct_render_occlusion_query_handle_t (*create_occlusion_query)();

    ct_render_occlusion_query_result_t
    (*get_result)(ct_render_occlusion_query_handle_t handle,
                  int32_t *_result);

    void (*destroy_occlusion_query)(ct_render_occlusion_query_handle_t handle);

    void (*set_palette_color)(uint8_t index,
                              const float rgba[4]);

    void (*set_view_name)(ct_render_view_id_t id,
                          const char *_name);

    void (*set_view_rect)(ct_render_view_id_t id,
                          uint16_t x,
                          uint16_t y,
                          uint16_t width,
                          uint16_t height);

    void (*set_view_scissor)(ct_render_view_id_t id,
                             uint16_t x,
                             uint16_t y,
                             uint16_t width,
                             uint16_t height);

    void (*set_view_clear)(ct_render_view_id_t id,
                           uint16_t flags,
                           uint32_t rgba,
                           float depth,
                           uint8_t stencil);

    void (*set_view_clear_mrt)(ct_render_view_id_t id,
                               uint16_t flags,
                               float depth,
                               uint8_t stencil,
                               uint8_t _0,
                               uint8_t _1,
                               uint8_t _2,
                               uint8_t _3,
                               uint8_t _4,
                               uint8_t _5,
                               uint8_t _6,
                               uint8_t _7);

    void (*set_view_mode)(ct_render_view_id_t id,
                          ct_render_view_mode_t mode);

    void (*set_view_frame_buffer)(ct_render_view_id_t id,
                                  ct_render_frame_buffer_handle_t handle);

    void (*set_view_transform)(ct_render_view_id_t id,
                               const void *_view,
                               const void *_proj);

    void (*set_view_transform_stereo)(ct_render_view_id_t id,
                                      const void *_view,
                                      const void *_projL,
                                      uint8_t flags,
                                      const void *_projR);

    void (*set_view_order)(ct_render_view_id_t id,
                           uint16_t num,
                           const ct_render_view_id_t *_order);

    void (*set_marker)(const char *_marker);

    void (*set_state)(uint64_t state,
                      uint32_t rgba);

    void (*set_condition)(ct_render_occlusion_query_handle_t handle,
                          bool visible);

    void (*set_stencil)(uint32_t fstencil,
                        uint32_t bstencil);

    uint16_t (*set_scissor)(uint16_t x,
                            uint16_t y,
                            uint16_t width,
                            uint16_t height);

    void (*set_scissor_cached)(uint16_t cache);

    uint32_t (*set_transform)(const void *_mtx,
                              uint16_t num);

    uint32_t (*alloc_transform)(ct_render_transform_t *_transform,
                                uint16_t num);

    void (*set_transform_cached)(uint32_t cache,
                                 uint16_t num);

    void (*set_uniform)(ct_render_uniform_handle_t handle,
                        const void *_value,
                        uint16_t num);

    void (*set_index_buffer)(ct_render_index_buffer_handle_t handle,
                             uint32_t firstIndex,
                             uint32_t numIndices);

    void
    (*set_dynamic_index_buffer)(ct_render_dynamic_index_buffer_handle_t handle,
                                uint32_t firstIndex,
                                uint32_t numIndices);

    void
    (*set_transient_index_buffer)(const ct_render_transient_index_buffer_t *_tib,
                                  uint32_t firstIndex,
                                  uint32_t numIndices);

    void (*set_vertex_buffer)(uint8_t stream,
                              ct_render_vertex_buffer_handle_t handle,
                              uint32_t startVertex,
                              uint32_t numVertices);

    void (*set_dynamic_vertex_buffer)(uint8_t stream,
                                      ct_render_dynamic_vertex_buffer_handle_t handle,
                                      uint32_t startVertex,
                                      uint32_t numVertices);

    void (*set_transient_vertex_buffer)(uint8_t stream,
                                        const ct_render_transient_vertex_buffer_t *_tvb,
                                        uint32_t startVertex,
                                        uint32_t numVertices);

    void
    (*set_instance_data_buffer)(const ct_render_instance_data_buffer_t *_idb,
                                uint32_t start,
                                uint32_t num);

    void
    (*set_instance_data_from_vertex_buffer)(ct_render_vertex_buffer_handle_t handle,
                                            uint32_t startVertex,
                                            uint32_t num);

    void
    (*set_instance_data_from_dynamic_vertex_buffer)(ct_render_dynamic_vertex_buffer_handle_t handle,
                                                    uint32_t startVertex,
                                                    uint32_t num);

    void (*set_texture)(uint8_t stage,
                        ct_render_uniform_handle_t sampler,
                        ct_render_texture_handle_t handle,
                        uint32_t flags);

    void (*touch)(ct_render_view_id_t id);

    void (*submit)(ct_render_view_id_t id,
                   ct_render_program_handle_t handle,
                   int32_t depth,
                   bool preserveState);

    void (*submit_occlusion_query)(ct_render_view_id_t id,
                                   ct_render_program_handle_t program,
                                   ct_render_occlusion_query_handle_t occlusionQuery,
                                   int32_t depth,
                                   bool preserveState);

    void (*submit_indirect)(ct_render_view_id_t id,
                            ct_render_program_handle_t handle,
                            ct_render_indirect_buffer_handle_t indirectHandle,
                            uint16_t start,
                            uint16_t num,
                            int32_t depth,
                            bool preserveState);

    void (*set_image)(uint8_t stage,
                      ct_render_texture_handle_t handle,
                      uint8_t mip,
                      ct_render_access_t access,
                      ct_render_texture_format_t format);

    void (*set_compute_index_buffer)(uint8_t stage,
                                     ct_render_index_buffer_handle_t handle,
                                     ct_render_access_t access);

    void (*set_compute_vertex_buffer)(uint8_t stage,
                                      ct_render_vertex_buffer_handle_t handle,
                                      ct_render_access_t access);

    void (*set_compute_dynamic_index_buffer)(uint8_t stage,
                                             ct_render_dynamic_index_buffer_handle_t handle,
                                             ct_render_access_t access);

    void (*set_compute_dynamic_vertex_buffer)(uint8_t stage,
                                              ct_render_dynamic_vertex_buffer_handle_t handle,
                                              ct_render_access_t access);

    void (*set_compute_indirect_buffer)(uint8_t stage,
                                        ct_render_indirect_buffer_handle_t handle,
                                        ct_render_access_t access);

    void (*dispatch)(ct_render_view_id_t id,
                     ct_render_program_handle_t handle,
                     uint32_t numX,
                     uint32_t numY,
                     uint32_t numZ,
                     uint8_t flags);

    void (*dispatch_indirect)(ct_render_view_id_t id,
                              ct_render_program_handle_t handle,
                              ct_render_indirect_buffer_handle_t indirectHandle,
                              uint16_t start,
                              uint16_t num,
                              uint8_t flags);

    void (*discard)();

    void (*blit)(ct_render_view_id_t id,
                 ct_render_texture_handle_t dst,
                 uint8_t dstMip,
                 uint16_t dstX,
                 uint16_t dstY,
                 uint16_t dstZ,
                 ct_render_texture_handle_t src,
                 uint8_t srcMip,
                 uint16_t srcX,
                 uint16_t srcY,
                 uint16_t srcZ,
                 uint16_t width,
                 uint16_t height,
                 uint16_t depth);

    void (*encoder_set_marker)(struct ct_render_encoder *_encoder,
                               const char *_marker);

    void (*encoder_set_state)(struct ct_render_encoder *_encoder,
                              uint64_t state,
                              uint32_t rgba);

    void (*encoder_set_condition)(struct ct_render_encoder *_encoder,
                                  ct_render_occlusion_query_handle_t handle,
                                  bool visible);

    void (*encoder_set_stencil)(struct ct_render_encoder *_encoder,
                                uint32_t fstencil,
                                uint32_t bstencil);

    uint16_t (*encoder_set_scissor)(struct ct_render_encoder *_encoder,
                                    uint16_t x,
                                    uint16_t y,
                                    uint16_t width,
                                    uint16_t height);

    void (*encoder_set_scissor_cached)(struct ct_render_encoder *_encoder,
                                       uint16_t cache);

    uint32_t (*encoder_set_transform)(struct ct_render_encoder *_encoder,
                                      const void *_mtx,
                                      uint16_t num);

    uint32_t (*encoder_alloc_transform)(struct ct_render_encoder *_encoder,
                                        ct_render_transform_t *_transform,
                                        uint16_t num);

    void (*encoder_set_transform_cached)(struct ct_render_encoder *_encoder,
                                         uint32_t cache,
                                         uint16_t num);

    void (*encoder_set_uniform)(struct ct_render_encoder *_encoder,
                                ct_render_uniform_handle_t handle,
                                const void *_value,
                                uint16_t num);

    void (*encoder_set_index_buffer)(struct ct_render_encoder *_encoder,
                                     ct_render_index_buffer_handle_t handle,
                                     uint32_t firstIndex,
                                     uint32_t numIndices);

    void (*encoder_set_dynamic_index_buffer)(struct ct_render_encoder *_encoder,
                                             ct_render_dynamic_index_buffer_handle_t handle,
                                             uint32_t firstIndex,
                                             uint32_t numIndices);

    void
    (*encoder_set_transient_index_buffer)(struct ct_render_encoder *_encoder,
                                          const ct_render_transient_index_buffer_t *_tib,
                                          uint32_t firstIndex,
                                          uint32_t numIndices);

    void (*encoder_set_vertex_buffer)(struct ct_render_encoder *_encoder,
                                      uint8_t stream,
                                      ct_render_vertex_buffer_handle_t handle,
                                      uint32_t startVertex,
                                      uint32_t numVertices);

    void
    (*encoder_set_dynamic_vertex_buffer)(struct ct_render_encoder *_encoder,
                                         uint8_t stream,
                                         ct_render_dynamic_vertex_buffer_handle_t handle,
                                         uint32_t startVertex,
                                         uint32_t numVertices);

    void
    (*encoder_set_transient_vertex_buffer)(struct ct_render_encoder *_encoder,
                                           uint8_t stream,
                                           const ct_render_transient_vertex_buffer_t *_tvb,
                                           uint32_t startVertex,
                                           uint32_t numVertices);

    void (*encoder_set_instance_data_buffer)(struct ct_render_encoder *_encoder,
                                             const ct_render_instance_data_buffer_t *_idb,
                                             uint32_t start,
                                             uint32_t num);

    void
    (*encoder_set_instance_data_from_vertex_buffer)(struct ct_render_encoder *_encoder,
                                                    ct_render_vertex_buffer_handle_t handle,
                                                    uint32_t startVertex,
                                                    uint32_t num);

    void
    (*encoder_set_instance_data_from_dynamic_vertex_buffer)(struct ct_render_encoder *_encoder,
                                                            ct_render_dynamic_vertex_buffer_handle_t handle,
                                                            uint32_t startVertex,
                                                            uint32_t num);

    void (*encoder_set_texture)(struct ct_render_encoder *_encoder,
                                uint8_t stage,
                                ct_render_uniform_handle_t sampler,
                                ct_render_texture_handle_t handle,
                                uint32_t flags);

    void (*encoder_touch)(struct ct_render_encoder *_encoder,
                          ct_render_view_id_t id);

    void (*encoder_submit)(struct ct_render_encoder *_encoder,
                           ct_render_view_id_t id,
                           ct_render_program_handle_t handle,
                           int32_t depth,
                           bool preserveState);

    void (*encoder_submit_occlusion_query)(struct ct_render_encoder *_encoder,
                                           ct_render_view_id_t id,
                                           ct_render_program_handle_t program,
                                           ct_render_occlusion_query_handle_t occlusionQuery,
                                           int32_t depth,
                                           bool preserveState);

    void (*encoder_submit_indirect)(struct ct_render_encoder *_encoder,
                                    ct_render_view_id_t id,
                                    ct_render_program_handle_t handle,
                                    ct_render_indirect_buffer_handle_t indirectHandle,
                                    uint16_t start,
                                    uint16_t num,
                                    int32_t depth,
                                    bool preserveState);

    void (*encoder_set_image)(struct ct_render_encoder *_encoder,
                              uint8_t stage,
                              ct_render_texture_handle_t handle,
                              uint8_t mip,
                              ct_render_access_t access,
                              ct_render_texture_format_t format);

    void (*encoder_set_compute_index_buffer)(struct ct_render_encoder *_encoder,
                                             uint8_t stage,
                                             ct_render_index_buffer_handle_t handle,
                                             ct_render_access_t access);

    void
    (*encoder_set_compute_vertex_buffer)(struct ct_render_encoder *_encoder,
                                         uint8_t stage,
                                         ct_render_vertex_buffer_handle_t handle,
                                         ct_render_access_t access);

    void
    (*encoder_set_compute_dynamic_index_buffer)(struct ct_render_encoder *_encoder,
                                                uint8_t stage,
                                                ct_render_dynamic_index_buffer_handle_t handle,
                                                ct_render_access_t access);

    void
    (*encoder_set_compute_dynamic_vertex_buffer)(struct ct_render_encoder *_encoder,
                                                 uint8_t stage,
                                                 ct_render_dynamic_vertex_buffer_handle_t handle,
                                                 ct_render_access_t access);

    void
    (*encoder_set_compute_indirect_buffer)(struct ct_render_encoder *_encoder,
                                           uint8_t stage,
                                           ct_render_indirect_buffer_handle_t handle,
                                           ct_render_access_t access);

    void (*encoder_dispatch)(struct ct_render_encoder *_encoder,
                             ct_render_view_id_t id,
                             ct_render_program_handle_t handle,
                             uint32_t numX,
                             uint32_t numY,
                             uint32_t numZ,
                             uint8_t flags);

    void (*encoder_dispatch_indirect)(struct ct_render_encoder *_encoder,
                                      ct_render_view_id_t id,
                                      ct_render_program_handle_t handle,
                                      ct_render_indirect_buffer_handle_t indirectHandle,
                                      uint16_t start,
                                      uint16_t num,
                                      uint8_t flags);

    void (*encoder_discard)(struct ct_render_encoder *_encoder);

    void (*encoder_blit)(struct ct_render_encoder *_encoder,
                         ct_render_view_id_t id,
                         ct_render_texture_handle_t dst,
                         uint8_t dstMip,
                         uint16_t dstX,
                         uint16_t dstY,
                         uint16_t dstZ,
                         ct_render_texture_handle_t src,
                         uint8_t srcMip,
                         uint16_t srcX,
                         uint16_t srcY,
                         uint16_t srcZ,
                         uint16_t width,
                         uint16_t height,
                         uint16_t depth);

    void (*request_screen_shot)(ct_render_frame_buffer_handle_t handle,
                                const char *_filePath);
};

CE_MODULE(ct_gfx_a0);

#endif //CETECH_GFX_H
//! \}
