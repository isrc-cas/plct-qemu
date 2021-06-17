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


static bool trans_aesesm(DisasContext *ctx, arg_aesesm *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs;
	bs = tcg_const_tl(a->bs);

	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);
#ifdef TARGET_RISCV32
	gen_helper_aes32esmi(source1, source1, source2, bs);
	gen_set_gpr(a->rs1, source1);
#endif

#ifdef TARGET_RISCV64
	TCGv dst = tcg_temp_new();
	gen_helper_aes64esm(dst, source1, source2, bs); 
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);
#endif
	tcg_temp_free(source1);
	tcg_temp_free(source2);
	tcg_temp_free(bs);
	return true;
}

static bool trans_aeses(DisasContext *ctx, arg_aeses *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs;
	bs = tcg_const_tl(a->bs);

	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);
#ifdef TARGET_RISCV32
	gen_helper_aes32esi(source1, source1, source2, bs);
	gen_set_gpr(a->rs1, source1);
#endif

#ifdef TARGET_RISCV64
	TCGv dst = tcg_temp_new();
	gen_helper_aes64es(dst, source1, source2, bs);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);
#endif
	tcg_temp_free(source1);
	tcg_temp_free(source2);
	tcg_temp_free(bs);
	return true;
}

static bool trans_aes32dsmi(DisasContext *ctx, arg_aes32dsmi *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs;
	bs = tcg_const_tl(a->bs);

	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);
	gen_helper_aes32dsmi(source1, source1, source2, bs);
	gen_set_gpr(a->rs1, source1);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	tcg_temp_free(bs);
	return true;
}

static bool trans_aes64dsm(DisasContext *ctx, arg_aes64dsm *a)
{
	REQUIRE_EXT(ctx, RVK);

	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_aes64dsm(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_aes64ks2(DisasContext *ctx, arg_aes64ks2 *a)
{
	REQUIRE_EXT(ctx, RVK);

	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_aes64ks2(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_aesds(DisasContext *ctx, arg_aesds *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs;
	bs = tcg_const_tl(a->bs);
	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);
#ifdef TARGET_RISCV32
	gen_helper_aes32dsi(source1, source1, source2, bs);
	gen_set_gpr(a->rs1, source1);
#endif

#ifdef TARGET_RISCV64
	TCGv dst = tcg_temp_new();
	gen_helper_aes64ds(dst, source1, source2, bs);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);
#endif
	tcg_temp_free(source1);
	tcg_temp_free(source2);
	tcg_temp_free(bs);
	return true;
}

static bool trans_aes64ks1i(DisasContext *ctx, arg_aes64ks1i *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv rcon;
	rcon = tcg_const_tl(a->rcon);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);

	TCGv dst = tcg_temp_new();
	gen_helper_aes64ks1i(dst, source1, rcon);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);
	
	tcg_temp_free(source1);
	tcg_temp_free(rcon);
	return true;
}

static bool trans_aes64im(DisasContext *ctx, arg_aes64im *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);

	TCGv dst = tcg_temp_new();
	gen_helper_aes64im(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha256sig0(DisasContext *ctx, arg_sha256sig0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);

	TCGv dst = tcg_temp_new();
	gen_helper_sha256sig0(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha256sig1(DisasContext *ctx, arg_sha256sig1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);

	TCGv dst = tcg_temp_new();
	gen_helper_sha256sig1(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha256sum0(DisasContext *ctx, arg_sha256sum0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	TCGv dst = tcg_temp_new();
	gen_helper_sha256sum0(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha256sum1(DisasContext *ctx, arg_sha256sum1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);

	TCGv dst = tcg_temp_new();
	gen_helper_sha256sum1(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha512sum0r(DisasContext *ctx, arg_sha512sum0r *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_sha512sum0r(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_sha512sum1r(DisasContext *ctx, arg_sha512sum1r *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_sha512sum1r(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_sha512sig0l(DisasContext *ctx, arg_sha512sig0l *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_sha512sig0l(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_sha512sig0h(DisasContext *ctx, arg_sha512sig0h *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);
	TCGv dst = tcg_temp_new();
	gen_helper_sha512sig0h(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_sha512sig1l(DisasContext *ctx, arg_sha512sig1l *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_sha512sig1l(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_sha512sig1h(DisasContext *ctx, arg_sha512sig1h *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);
	TCGv dst = tcg_temp_new();
	gen_helper_sha512sig1h(dst, source1, source2);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	tcg_temp_free(source2);
	return true;
}

static bool trans_sha512sig0(DisasContext *ctx, arg_sha512sig0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	TCGv dst = tcg_temp_new();
	gen_helper_sha512sig0(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha512sig1(DisasContext *ctx, arg_sha512sig1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	TCGv dst = tcg_temp_new();
	gen_helper_sha512sig1(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha512sum0(DisasContext *ctx, arg_sha512sum0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	TCGv dst = tcg_temp_new();
	gen_helper_sha512sum0(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

static bool trans_sha512sum1(DisasContext *ctx, arg_sha512sum1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	TCGv dst = tcg_temp_new();
	gen_helper_sha512sum1(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

/* SM3 */
static bool trans_sm3p0(DisasContext *ctx, arg_sm3p0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	TCGv dst = tcg_temp_new();
	gen_helper_sm3p0(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);
	
	
	tcg_temp_free(source1);
	return true;
}
static bool trans_sm3p1(DisasContext *ctx, arg_sm3p1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv source1 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	TCGv dst = tcg_temp_new();
	gen_helper_sm3p1(dst, source1);
	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);

	tcg_temp_free(source1);
	return true;
}

/* SM4 */
static bool trans_sm4ed(DisasContext *ctx, arg_sm4ed *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs = tcg_const_tl(a->bs);

	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_sm4ed(dst, source2, source1, bs);

	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);
	tcg_temp_free(source1);
	tcg_temp_free(source2);
	tcg_temp_free(bs);
	return true;
}

static bool trans_sm4ks(DisasContext *ctx, arg_sm4ks *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs = tcg_const_tl(a->bs);

	TCGv source1 = tcg_temp_new();
	TCGv source2 = tcg_temp_new();
	gen_get_gpr(source1, a->rs1);
	gen_get_gpr(source2, a->rs2);

	TCGv dst = tcg_temp_new();
	gen_helper_sm4ks(dst, source2, source1, bs);

	gen_set_gpr(a->rd, dst);
	tcg_temp_free(dst);
	tcg_temp_free(source1);
	tcg_temp_free(source2);
	tcg_temp_free(bs);
	return true;
}
