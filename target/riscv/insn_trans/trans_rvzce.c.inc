/*
 * RISC-V translation routines for the RV64M Standard Extension.
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

static bool gen_unary(DisasContext *ctx, arg_zcea_unary *a, void (*func)(TCGv, TCGv))
{
    TCGv source = tcg_temp_new();
    gen_get_gpr(source, a->rs1_3);

    (*func)(source, source);
    gen_set_gpr(a->rs1_3, source);
    tcg_temp_free(source);
    return true;
}

static bool trans_c_zext_b(DisasContext *ctx, arg_c_zext_b *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    return gen_unary(ctx, a, tcg_gen_ext8u_tl);
}

static bool trans_c_zext_h(DisasContext *ctx, arg_c_zext_h *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    return gen_unary(ctx, a, tcg_gen_ext16u_tl);
}

static bool trans_c_sext_b(DisasContext *ctx, arg_c_sext_b *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    return gen_unary(ctx, a, tcg_gen_ext8s_tl);
}

static bool trans_c_sext_h(DisasContext *ctx, arg_c_sext_h *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    return gen_unary(ctx, a, tcg_gen_ext16s_tl);
}

static bool trans_c_not(DisasContext *ctx, arg_c_not *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    tcg_gen_not_tl(cpu_gpr[a->rs1_3], cpu_gpr[a->rs1_3]);
    return true;
}

static bool trans_c_neg(DisasContext *ctx, arg_c_neg *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    tcg_gen_neg_tl(cpu_gpr[a->rs1_3], cpu_gpr[a->rs1_3]);
    return true;
}

static bool trans_c_mul(DisasContext *ctx, arg_c_mul *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    tcg_gen_mul_tl(cpu_gpr[a->rs1_3], cpu_gpr[a->rs1_3], cpu_gpr[a->rs2_3]);
    return true;
}

static bool gen_branch_zce(DisasContext *ctx, arg_zcea_b *a, TCGCond cond)
{
    TCGLabel *l = gen_new_label();
    TCGv source = tcg_temp_new();
    gen_get_gpr(source, a->rs1);
    TCGv uimm = tcg_const_tl(a->zce_uimm);

    tcg_gen_brcond_tl(cond, source, uimm, l);
    gen_goto_tb(ctx, 1, ctx->pc_succ_insn);
    gen_set_label(l); /* branch taken */

    if (!has_ext(ctx, RVZCE) && ((ctx->base.pc_next + a->zce_offset) & 0x3))
    {
        /* misaligned */
        gen_exception_inst_addr_mis(ctx);
    }
    else
    {
        gen_goto_tb(ctx, 0, ctx->base.pc_next + a->zce_offset);
    }
    ctx->base.is_jmp = DISAS_NORETURN;

    tcg_temp_free(source);
    tcg_temp_free(uimm);

    return true;
}

static bool trans_beqi(DisasContext *ctx, arg_beqi *a)
{
    return gen_branch_zce(ctx, a, TCG_COND_EQ);
}

static bool trans_bnei(DisasContext *ctx, arg_bnei *a)
{
    return gen_branch_zce(ctx, a, TCG_COND_NE);
}

static bool trans_decbnez(DisasContext *ctx, arg_decbnez *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv source = tcg_temp_new();
    gen_get_gpr(source, a->rd);
    TCGv scale = tcg_const_tl(1 << a->zce_scale);
    tcg_gen_sub_tl(source, source, scale);

    // branch
    TCGLabel *l = gen_new_label();
    TCGCond cond = TCG_COND_NE;
    tcg_gen_brcond_tl(cond, source, 0, l);
    gen_goto_tb(ctx, 1, ctx->pc_succ_insn);
    gen_set_label(l); /* branch taken */

    if (!has_ext(ctx, RVZCE) && ((ctx->base.pc_next + a->zce_b_imm_dec) & 0x3))
    {
        /* misaligned */
        gen_exception_inst_addr_mis(ctx);
    }
    else
    {
        gen_goto_tb(ctx, 0, ctx->base.pc_next + a->zce_b_imm_dec);
    }
    ctx->base.is_jmp = DISAS_NORETURN;

    tcg_temp_free(source);
    tcg_temp_free(scale);

    return true;
}

static bool trans_lwgp(DisasContext *ctx, arg_lwgp *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    gen_get_gpr(t0, xGP);
    tcg_gen_addi_tl(t0, t0, a->zce_b_imm_lw);

    MemOp memop = MO_TESL; // 32s-bit
    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, memop);
    gen_set_gpr(a->rd, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_swgp(DisasContext *ctx, arg_swgp *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();
    gen_get_gpr(t0, xGP);
    tcg_gen_addi_tl(t0, t0, a->zce_b_imm_sw);
    gen_get_gpr(dat, a->rs2);

    MemOp memop = MO_TESL; // 32s-bit
    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, memop);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}

static bool trans_c_tblj_all(DisasContext *ctx, arg_c_tblj_all *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv index = tcg_const_tl(a->zce_table_jump_index);

    gen_helper_c_tblj_all(cpu_env, index);

    tcg_temp_free(index);
    return true;
}

static bool trans_c_decbnez(DisasContext *ctx, arg_c_decbnez *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv source = tcg_temp_new();
    gen_get_gpr(source, a->rs1_3);
    TCGv scale = tcg_const_tl(1 << a->zce_scale);
    tcg_gen_sub_tl(source, source, scale);

    // branch
    TCGLabel *l = gen_new_label();
    TCGCond cond = TCG_COND_NE;
    tcg_gen_brcond_tl(cond, source, 0, l);
    gen_goto_tb(ctx, 1, ctx->pc_succ_insn);
    gen_set_label(l); /* branch taken */

    if (!has_ext(ctx, RVZCE) && ((ctx->base.pc_next - a->zce_nzuimm) & 0x3))
    {
        /* misaligned */
        gen_exception_inst_addr_mis(ctx);
    }
    else
    {
        gen_goto_tb(ctx, 0, ctx->base.pc_next - a->zce_nzuimm);
    }
    ctx->base.is_jmp = DISAS_NORETURN;

    tcg_temp_free(source);
    tcg_temp_free(scale);
    return true;
}

static bool trans_c_lbu(DisasContext *ctx, arg_c_lbu *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1_3);
    tcg_gen_addi_tl(t0, t0, a->zce_b_uimm);

    MemOp memop = MO_UB; // 8u-bit
    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, memop);
    gen_set_gpr(a->rs2_3, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_lhu(DisasContext *ctx, arg_c_lhu *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1_3);
    tcg_gen_addi_tl(t0, t0, a->zce_h_uimm);

    MemOp memop = MO_UW; // 16u-bit
    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, memop);
    gen_set_gpr(a->rs2_3, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_lb(DisasContext *ctx, arg_c_lb *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1_3);
    tcg_gen_addi_tl(t0, t0, a->zce_b_uimm);

    MemOp memop = MO_SB; // 8s-bit
    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, memop);
    gen_set_gpr(a->rs2_3, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_lh(DisasContext *ctx, arg_c_lh *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1_3);
    tcg_gen_addi_tl(t0, t0, a->zce_h_uimm);

    MemOp memop = MO_SW; // 16s-bit
    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, memop);
    gen_set_gpr(a->rs2_3, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_sb(DisasContext *ctx, arg_c_sb *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();
    gen_get_gpr(t0, a->rs1_3);
    tcg_gen_addi_tl(t0, t0, a->zce_b_uimm);
    gen_get_gpr(dat, a->rs2_3);

    MemOp memop = MO_UB; // 8u-bit
    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, memop);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}

static bool trans_c_sh(DisasContext *ctx, arg_c_sh *a)
{
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();
    gen_get_gpr(t0, a->rs1_3);
    tcg_gen_addi_tl(t0, t0, a->zce_h_uimm);
    gen_get_gpr(dat, a->rs2_3);

    MemOp memop = MO_UW; // 16u-bit
    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, memop);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}

static bool trans_c_popret(DisasContext *ctx, arg_c_popret *a)
{
    REQUIRE_EXT(ctx, RVZCE);

    TCGv sp = tcg_temp_new();
    gen_get_gpr(sp, xSP);

    TCGv spimm0_5 = tcg_const_tl(a->zce_spimm0_5);
    TCGv ret0_1 = tcg_const_tl(a->zce_ret0_1);
    TCGv rlist3 = tcg_const_tl(a->zce_rlist3);

    gen_helper_c_popret(cpu_env, sp, spimm0_5, rlist3, ret0_1);
    tcg_temp_free(sp);
    tcg_temp_free(spimm0_5);
    tcg_temp_free(ret0_1);
    tcg_temp_free(rlist3);
    return true;
}

static bool trans_c_popret_e(DisasContext *ctx, arg_c_popret_e *a)
{
    REQUIRE_EXT(ctx, RVZCE);

    TCGv sp = tcg_temp_new();
    gen_get_gpr(sp, xSP);

    TCGv spimm0_5 = tcg_const_tl(a->zce_spimm0_5);
    TCGv ret0_2 = tcg_const_tl(a->zce_ret0_2);
    TCGv rlist2 = tcg_const_tl(a->zce_rlist2);

    gen_helper_c_popret_e(cpu_env, sp, spimm0_5, rlist2, ret0_2);
    tcg_temp_free(sp);
    tcg_temp_free(spimm0_5);
    tcg_temp_free(ret0_2);
    tcg_temp_free(rlist2);
    return true;
}

static bool trans_c_pop(DisasContext *ctx, arg_c_pop *a)
{
    REQUIRE_EXT(ctx, RVZCE);

    TCGv sp = tcg_temp_new();
    gen_get_gpr(sp, xSP);

    TCGv spimm = tcg_const_tl(a->zce_spimm);
    TCGv rlist3 = tcg_const_tl(a->zce_rlist3);

    gen_helper_c_pop(cpu_env, sp, spimm, rlist3);
    tcg_temp_free(sp);
    tcg_temp_free(spimm);
    tcg_temp_free(rlist3);
    return true;
}

static bool trans_c_pop_e(DisasContext *ctx, arg_c_pop_e *a)
{
    REQUIRE_EXT(ctx, RVZCE);

    TCGv sp = tcg_temp_new();
    gen_get_gpr(sp, xSP);

    TCGv spimm = tcg_const_tl(a->zce_spimm);
    TCGv rlist2 = tcg_const_tl(a->zce_rlist2);

    gen_helper_c_pop_e(cpu_env, sp, spimm, rlist2);
    tcg_temp_free(sp);
    tcg_temp_free(spimm);
    tcg_temp_free(rlist2);
    return true;
}

static bool trans_c_push(DisasContext *ctx, arg_c_push *a)
{
    REQUIRE_EXT(ctx, RVZCE);

    TCGv sp = tcg_temp_new();
    gen_get_gpr(sp, xSP);

    TCGv rlist = tcg_const_tl(a->zce_rlist3);
    TCGv spimm = tcg_const_tl(a->zce_spimm0_5);

    gen_helper_c_push(cpu_env, sp, spimm, rlist);
    tcg_temp_free(sp);
    tcg_temp_free(rlist);
    tcg_temp_free(spimm);
    return true;
}

static bool trans_c_push_e(DisasContext *ctx, arg_c_push_e *a)
{
    REQUIRE_EXT(ctx, RVZCE);

    TCGv sp = tcg_temp_new();
    gen_get_gpr(sp, xSP);

    TCGv rlist = tcg_const_tl(a->zce_rlist2);
    TCGv spimm = tcg_const_tl(a->zce_spimm_pe);

    gen_helper_c_push_e(cpu_env, sp, spimm, rlist);
    tcg_temp_free(sp);
    tcg_temp_free(rlist);
    tcg_temp_free(spimm);
    return true;
}
//#undef XLEN

// RV64 Only
static bool trans_c_zext_w(DisasContext *ctx, arg_c_zext_w *a)
{
    REQUIRE_64BIT(ctx);
    REQUIRE_EXT(ctx, RVZCE);
    return gen_unary(ctx, a, tcg_gen_ext32u_tl);
}

static bool trans_ldgp(DisasContext *ctx, arg_ldgp *a)
{
    REQUIRE_64BIT(ctx);
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    gen_get_gpr(t0, xGP);
    tcg_gen_addi_tl(t0, t0, a->zce_b_imm_ld);

    MemOp memop = MO_TEQ; // 64s-bit
    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, memop);
    gen_set_gpr(a->rd, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_sdgp(DisasContext *ctx, arg_sdgp *a)
{
    REQUIRE_64BIT(ctx);
    REQUIRE_EXT(ctx, RVZCE);
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();
    gen_get_gpr(t0, xGP);
    tcg_gen_addi_tl(t0, t0, a->zce_b_imm_sd);
    gen_get_gpr(dat, a->rs2);

    MemOp memop = MO_TEQ; // 64s-bit
    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, memop);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}
