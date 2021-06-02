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

#define X_S0 8
#define X_Sn 16
#define XLEN (8 * sizeof(target_ulong))
#define align16(x) (((x) + 15) & ~0xf)
#define ZCE_POP(env, sp, bytes, rlist, ra, spimm, ret_val, ret) \
{ \
    target_ulong stack_adjust = rlist * (XLEN >> 3); \
    stack_adjust += (ra == 1) ? XLEN >> 3 : 0; \
    stack_adjust = align16(stack_adjust) + spimm; \
    target_ulong addr = sp + stack_adjust; \
    if(ra) \
    { \
        addr -= bytes; \
        switch (bytes) \
        { \
        case 4: \
            env->gpr[xRA] = (int)(addr); \
            break; \
        case 8: \
            env->gpr[xRA] = (long)(addr); \
            break; \
        default: \
            break; \
        } \
    } \
    for (int i = 0; i < rlist; i++) \
    { \
        addr -= bytes; \
        target_ulong reg = i < 2 ? X_S0 + i : X_Sn + i; \
        switch (bytes) \
        { \
        case 4: \
            env->gpr[reg] = (int)(addr); \
            break; \
        case 8: \
            env->gpr[reg] = (long)(addr); \
            break; \
        default: \
            break; \
        } \
    } \
    switch (ret_val) \
    { \
    case 1: \
        env->gpr[xA0] = 0; \
        break; \
    case 2: \
        env->gpr[xA0] = 1; \
        break; \
    case 3: \
        env->gpr[xA0] = -1; \
        break; \
    default: \
        break; \
    } \
    env->gpr[xSP] = sp + stack_adjust; \
    if(ret) \
    { \
        env->pc = env->gpr[xRA]; \
    } \
}

#define ZCE_PUSH(env, sp, bytes, rlist, ra, spimm, alist) \
{ \
target_ulong addr = sp; \
if (ra) { \
  addr -= bytes; \
  switch (bytes) { \
  case 4: \
    env->gpr[xRA] = (int)(addr); \
    break; \
  case 8: \
    env->gpr[xRA] = (long)(addr); \
    break; \
  default: \
    break; \
  } \
} \
target_ulong data; \
for (int i=0; i < rlist; i++) { \
  addr -= bytes; \
  data = i < 2 ? X_S0 + i : X_Sn + i; \
  switch (bytes) { \
  case 4: \
    env->gpr[addr] = data; \
    break; \
  case 8: \
    env->gpr[addr] = data; \
    break; \
  default: \
    break; \
  } \
} \
for (int i=0; i < alist; i++) { \
   env->gpr[i < 2 ? X_S0 + i : X_Sn + i] = (long)(env->gpr[xA0 + i]); \
} \
target_ulong stack_adjust = align16(addr - sp) - spimm; \
env -> gpr[xSP] = sp + stack_adjust; \
}

void HELPER(c_tblj_all)(target_ulong index)
{
    /*
    if (index < 7)
    {
        // C.TBLJALM
    }
    else if (index >= 8 && index < 64)
    {
        // C.TBLJ
    }
    else if (index > 64)
    {
        // C.TBLJAL
    }
    */
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
    if(rlist == 3) {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    ZCE_POP(env, sp, bytes, rlist, true, spimm, 0, false);
}
/*
void HELPER(c_popret)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist, target_long ret)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    ZCE_POP(env, sp, bytes, rlist, true, spimm, ret, true);
}

void HELPER(c_popret_e)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist, target_long ret)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    if(rlist == 3) {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    ZCE_POP(env, sp, bytes, rlist, true, spimm, ret, true);
}
*/
void HELPER(c_push)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    target_ulong alist = rlist <= 4 ? rlist:4;
    ZCE_PUSH(env, sp, bytes, rlist, true, spimm, alist);
}

void HELPER(c_push_e)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    if(rlist == 3) {
        return;
    }
    ZCE_PUSH(env, sp, bytes, rlist, true, spimm, 0);
}

#undef X_S0
#undef X_Sn
#undef align16
#undef XLEN
#undef ZCE_POP