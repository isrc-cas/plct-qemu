/*
 * RISC-V Emulation Helpers for QEMU.
 *
 * Copyright (c) 2016-2017 Sagar Karandikar, sagark@eecs.berkeley.edu
 * Copyright (c) 2017-2018 SiFive, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/exec-all.h"
#include "exec/helper-proto.h"
// #include "exec/memop.h"
// #include "exec/memattrs.h"
// #include "tcg/tcg-internal.h"
#include "exec/cpu_ldst.h"

#define X_S0 8
#define X_Sn 16
#define XLEN (8 * sizeof(target_ulong))
#define align16(x) (((x) + 15) & ~0xf)
#define sext_xlen(x) (((int64_t)(x) << (64 - XLEN)) >> (64 - XLEN))

#define load32 cpu_ldsw_le_data_ra(env, t1, t0)
#define load64 cpu_ldl_le_data_ra(env, t1, t0)
#define loadu32 cpu_lduw_le_data_ra(env, t1, t0)

#define store32 cpu_stw_le_data(env, dat, t0)
#define store64 cpu_stl_le_data(env, dat, t0)

#define ZCE_POP(env, sp, bytes, rlist, ra, spimm, ret_val, ret) \
    {                                                           \
        target_ulong stack_adjust = rlist * (XLEN >> 3);        \
        stack_adjust += (ra == 1) ? XLEN >> 3 : 0;              \
        stack_adjust = align16(stack_adjust) + spimm;           \
        target_ulong addr = sp + stack_adjust;                  \
        target_ulong t1 = 0;                                        \
        target_ulong t0 = addr;                                 \
        if (ra)                                                 \
        {                                                       \
            addr -= bytes;                                      \
            switch (bytes)                                      \
            {                                                   \
            case 4:                                             \
                load32;                            \
                env->gpr[xRA] = t1;                             \
                break;                                          \
            case 8:                                             \
                load64;                            \
                env->gpr[xRA] = t1;                             \
                break;                                          \
            default:                                            \
                break;                                          \
            }                                                   \
        }                                                       \
        for (int i = 0; i < rlist; i++)                         \
        {                                                       \
            addr -= bytes;                                      \
            t0 = addr;                                          \
            target_ulong reg = i < 2 ? X_S0 + i : X_Sn + i;     \
            switch (bytes)                                      \
            {                                                   \
            case 4:                                             \
                load32;                            \
                env->gpr[reg] = t1;                             \
                break;                                          \
            case 8:                                             \
                load64;                            \
                env->gpr[reg] = t1;                             \
                break;                                          \
            default:                                            \
                break;                                          \
            }                                                   \
        }                                                       \
        switch (ret_val)                                        \
        {                                                       \
        case 1:                                                 \
            env->gpr[xA0] = 0;                                  \
            break;                                              \
        case 2:                                                 \
            env->gpr[xA0] = 1;                                  \
            break;                                              \
        case 3:                                                 \
            env->gpr[xA0] = -1;                                 \
            break;                                              \
        default:                                                \
            break;                                              \
        }                                                       \
        env->gpr[xSP] = sp + stack_adjust;                      \
        if (ret)                                                \
        {                                                       \
            env->pc = env->gpr[xRA];                            \
        }                                                       \
    }

#define ZCE_PUSH(env, sp, bytes, rlist, ra, spimm, alist)                      \
    {                                                                          \
        target_ulong addr = sp;                                                \
        target_ulong dat;                                                      \
        target_ulong t0;                                                       \
        if (ra)                                                                \
        {                                                                      \
            addr -= bytes;                                                     \
            t0 = addr;                                                         \
            dat = env->gpr[xRA];                                               \
            switch (bytes)                                                     \
            {                                                                  \
            case 4:                                                            \
                store32;                                                       \
                break;                                                         \
            case 8:                                                            \
                store64;                                                       \
                break;                                                         \
            default:                                                           \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        target_ulong data;                                                     \
        for (int i = 0; i < rlist; i++)                                        \
        {                                                                      \
            addr -= bytes;                                                     \
            data = i < 2 ? X_S0 + i : X_Sn + i;                                \
            t0 = addr;                                                         \
            dat = data;                                                        \
            switch (bytes)                                                     \
            {                                                                  \
            case 4:                                                            \
                store32;                                                       \
                break;                                                         \
            case 8:                                                            \
                store64;                                                       \
                break;                                                         \
            default:                                                           \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        for (int i = 0; i < alist; i++)                                        \
        {                                                                      \
            env->gpr[i < 2 ? X_S0 + i : X_Sn + i] = (long)(env->gpr[xA0 + i]); \
        }                                                                      \
        target_ulong stack_adjust = align16(addr - sp) - spimm;                \
        env->gpr[xSP] = sp + stack_adjust;                                     \
    }

void HELPER(c_tblj_all)(CPURISCVState *env, target_ulong csr, target_ulong index)
{
#ifndef CONFIG_USER_ONLY
    target_ulong val = 0;
    RISCVException ret =
        (env->priv == PRV_U) ? read_zce_tblj_csr_u(env, csr, &val) : (env->priv == PRV_S) ? read_zce_tblj_csr_s(env, csr, &val)
                                                                                          : read_zce_tblj_csr_m(env, csr, &val);

    if (ret != RISCV_EXCP_NONE)
    {
        riscv_raise_exception(env, ret, GETPC());
    }

    uint8_t mode = get_field(val, TBLJALVEC_MODE);
    uint8_t scale = get_field(val, TBLJALVEC_SCALE);
    target_ulong base = get_field(val, TBLJALVEC_BASE);
    target_ulong target;
    target_ulong next_pc = ctx->base.pc_next + imm;
    target_ulong t1 = 0;
    target_ulong t0;
    switch (mode)
    {
    case 0: //jump table mode
        if (XLEN == 32)
        {
            t0 = base + index << 2;
            loadu32;
            target = t1;
        }
        else // XLEN = 8
        {
            t0 = base + index << 3;
            load64;
            target = t1;
        }
        if (index < 7) // t0
        {
            // C.TBLJALM
            env->gpr[5] = next_pc;
        }
        else if (index < 64) // zero
        {
            // C.TBLJ
            env->gpr[0] = next_pc;
        }
        else // ra
        {
            // C.TBLJAL
            env->gpr[1] = next_pc;
        }
        break;
    case 1: // TODO: Vector Table Mode
        break;
    case 2: // TODO: Emulation Mode
        break;
    default:
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
        break;
    }
#endif
    return;
}

void HELPER(c_pop)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    ZCE_POP(env, sp, bytes, rlist, true, spimm, 0, false);
}

void HELPER(c_pop_e)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    if (rlist == 3)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    ZCE_POP(env, sp, bytes, rlist, true, spimm, 0, false);
}

void HELPER(c_popret)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist, target_ulong ret)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    ZCE_POP(env, sp, bytes, rlist, true, spimm, ret, true);
}

void HELPER(c_popret_e)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist, target_ulong ret)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    if (rlist == 3)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    ZCE_POP(env, sp, bytes, rlist, true, spimm, ret, true);
}

void HELPER(c_push)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    target_ulong alist = rlist <= 4 ? rlist : 4;
    ZCE_PUSH(env, sp, bytes, rlist, true, spimm, alist);
}

void HELPER(c_push_e)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    if (rlist == 3)
    {
        return;
    }
    ZCE_PUSH(env, sp, bytes, rlist, true, spimm, 0);
}

#undef X_S0
#undef X_Sn
#undef align16
#undef XLEN
#undef ZCE_POP
#undef sext_xlen

#undef load32
#undef loadu32
#undef load64

#undef store32
#undef store64