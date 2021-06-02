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

/*
void HELPER(c_tblj_all)(target_ulong index)
{
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
}
*/

#define XLEN (8 * sizeof(target_ulong))
#define align16(x) (((x) + 15) & ~0xf)
void zce_pop(CPURISCVState *env, target_ulong sp, target_ulong bytes, target_ulong rilst, target_ulong ra, target_ulong spimm, target_ulong ret_val, target_ulong ret)
{
    target_ulong data;
    target_ulong stack_adjust = rlist * (XLEN >> 3);
    stack_adjust += (ra == 1) ? XLEN >> 3 : 0;
    stack_adjust = align16(stack_adjust) + spimm;
    target_ulong addr = sp + stack_adjust;
    // /*
    if (ra)
    {
        addr -= bytes;
        switch (bytes)
        {
        case 4:
            // WRITE_REG(X_RA, MMU.load_int32(addr));
            // &env->xRA
            break;
        case 8:
            // WRITE_REG(X_RA, MMU.load_int64(addr));
            break;
        default:
            break;
        }
    }

    for (target_ulong i = 0; i < rlist; i++)
    {
        addr -= bytes;
        target_ulong reg = i < 2 ? X_S0 + i : X_Sn + i;
        switch (bytes)
        {
        case 4:
            // WRITE_REG(reg, MMU.load_int32(addr));
            break;
        case 8:
            // WRITE_REG(reg, MMU.load_int64(addr));
            break;
        default:
            break;
        }
    }

    WRITE_REG(X_SP, SP + stack_adjust);
    switch (ret_val)
    {
    case 1:
        WRITE_REG(X_A0, 0);
        break;
    case 2:
        WRITE_REG(X_A0, 1);
        break;
    case 3:
        WRITE_REG(X_A0, -1);
        break;
    default:
        break;
    }

    if (ret)
        set_pc(RA)
    // */
}
/*
void HELPER(c_popret)(target_ulong spimm, target_ulong ret, target_ulong rlist)
{
}

void HELPER(c_popret_e)(target_ulong spimm, target_ulong ret, target_ulong rlist) {}
void HELPER(c_pop_e)(target_ulong spimm, target_ulong rlist) {}
void HELPER(c_push)(target_ulong spimm, target_ulong rlist) {}
void HELPER(c_push_e)(target_ulong spimm, target_ulong rlist) {}
*/

void HELPER(c_pop)(CPURISCVState *env, target_ulong sp, target_ulong spimm, target_ulong rlist)
{
    if (((XLEN / 4 - 1) & sp) != 0)
    {
        return;
    }
    target_ulong bytes = XLEN >> 3;
    zce_pop(env, sp, bytes, rlist, 1, spimm, 0, 0);
}

#undef align16
#undef XLEN