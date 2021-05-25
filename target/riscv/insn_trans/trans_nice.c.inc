/*
 * RISC-V translation routines for the Custom NICE Instruction Set.
 *
 * Copyright (c) 2016-2017 Sagar Karandikar, sagark@eecs.berkeley.edu
 * Copyright (c) 2018 Peer Adelt, peer.adelt@hni.uni-paderborn.de
 *                    Bastian Koppelmann, kbastian@mail.uni-paderborn.de
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

#define REQUIRE_NICE do {\
    if (ctx->ext_nice == 0) \
        return false;                       \
} while (0)

static bool trans_custom_lbuf(DisasContext *ctx, arg_custom_lbuf *a)
{
    REQUIRE_NICE;
    gen_helper_custom_lbuf(cpu_env, cpu_gpr[a->rs1]);
    return true;
}

static bool trans_custom_sbuf(DisasContext *ctx, arg_custom_sbuf *a)
{
    REQUIRE_NICE;
    gen_helper_custom_sbuf(cpu_env, cpu_gpr[a->rs1]);
    return true;
}

static bool trans_custom_rowsum(DisasContext *ctx, arg_custom_rowsum *a)
{
    REQUIRE_NICE;
    gen_helper_custom_rowsum(cpu_gpr[a->rd], cpu_env, cpu_gpr[a->rs1]);
    return true;
}