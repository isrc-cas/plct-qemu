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

#ifdef TARGET_RISCV32
	gen_helper_aes32esmi(cpu_gpr[a->rs1], cpu_gpr[a->rs1], cpu_gpr[a->rs2], bs);
#endif

#ifdef TARGET_RISCV64
	gen_helper_aes64esm(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2], cpu_gpr[a->rd], bs);
#endif

	tcg_temp_free(bs);
	return true;
}

static bool trans_aeses(DisasContext *ctx, arg_aeses *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs;
	bs = tcg_const_tl(a->bs);

#ifdef TARGET_RISCV32
	gen_helper_aes32esi(cpu_gpr[a->rs1], cpu_gpr[a->rs1], cpu_gpr[a->rs2], bs);
#endif

#ifdef TARGET_RISCV64
	gen_helper_aes64es(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2], cpu_gpr[a->rd], bs);
#endif

	tcg_temp_free(bs);
	return true;
}

static bool trans_aesdsm_ks2(DisasContext *ctx, arg_aesdsm_ks2 *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs;
	bs = tcg_const_tl(a->bs);

#ifdef TARGET_RISCV32
	gen_helper_aes32dsmi(cpu_gpr[a->rs1], cpu_gpr[a->rs1], cpu_gpr[a->rs2], bs);
#endif

#ifdef TARGET_RISCV64
	gen_helper_aes64dsm_ks2(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2], cpu_gpr[a->rd], bs);
#endif

	tcg_temp_free(bs);
	return true;
}

static bool trans_aesds(DisasContext *ctx, arg_aesds *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs;
	bs = tcg_const_tl(a->bs);

#ifdef TARGET_RISCV32
	gen_helper_aes32dsi(cpu_gpr[a->rs1], cpu_gpr[a->rs1], cpu_gpr[a->rs2], bs);
#endif

#ifdef TARGET_RISCV64
	gen_helper_aes64ds(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2], cpu_gpr[a->rd], bs);
#endif

	tcg_temp_free(bs);
	return true;
}

#ifdef TARGET_RISCV64
static bool trans_aes64ks1i(DisasContext *ctx, arg_aes64ks1i *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv rcon;
	rcon = tcg_const_tl(a->rcon);
	gen_helper_aes64ks1i(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd], rcon);
	tcg_temp_free(rcon);
	return true;
}

static bool trans_aes64im(DisasContext *ctx, arg_aes64im *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_aes64im(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}
#endif

static bool trans_sha256sig0(DisasContext *ctx, arg_sha256sig0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha256sig0(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha256sig1(DisasContext *ctx, arg_sha256sig1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha256sig1(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha256sum0(DisasContext *ctx, arg_sha256sum0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha256sum0(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha256sum1(DisasContext *ctx, arg_sha256sum1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha256sum1(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sum0r(DisasContext *ctx, arg_sha512sum0r *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sum0r(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2],cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sum1r(DisasContext *ctx, arg_sha512sum1r *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sum1r(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2],cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sig0l(DisasContext *ctx, arg_sha512sig0l *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sig0l(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2],cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sig0h(DisasContext *ctx, arg_sha512sig0h *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sig0h(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2],cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sig1l(DisasContext *ctx, arg_sha512sig1l *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sig1l(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2],cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sig1h(DisasContext *ctx, arg_sha512sig1h *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sig1h(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rs2],cpu_gpr[a->rd]);
	return true;
}

#ifdef TARGET_RISCV64
static bool trans_sha512sig0(DisasContext *ctx, arg_sha512sig0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sig0(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sig1(DisasContext *ctx, arg_sha512sig1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sig1(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sum0(DisasContext *ctx, arg_sha512sum0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sum0(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}

static bool trans_sha512sum1(DisasContext *ctx, arg_sha512sum1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sha512sum1(cpu_gpr[a->rd], cpu_gpr[a->rs1], cpu_gpr[a->rd]);
	return true;
}
#endif

/* SM3 */
static bool trans_sm3p0(DisasContext *ctx, arg_sm3p0 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sm3p0(cpu_gpr[a->rd], cpu_gpr[a->rs1]);
	return true;
}
static bool trans_sm3p1(DisasContext *ctx, arg_sm3p1 *a)
{
	REQUIRE_EXT(ctx, RVK);
	gen_helper_sm3p1(cpu_gpr[a->rd], cpu_gpr[a->rs1]);
	return true;
}

/* SM4 */
static bool trans_sm4ed(DisasContext *ctx, arg_sm4ed *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs = tcg_const_tl(a->bs);

	gen_helper_sm4ed(cpu_gpr[a->rs1], cpu_gpr[a->rs2], cpu_gpr[a->rs1], bs);

	tcg_temp_free(bs);
	return true;
}

static bool trans_sm4ks(DisasContext *ctx, arg_sm4ks *a)
{
	REQUIRE_EXT(ctx, RVK);
	TCGv bs = tcg_const_tl(a->bs);

	gen_helper_sm4ks(cpu_gpr[a->rs1], cpu_gpr[a->rs2], cpu_gpr[a->rs1], bs);

	tcg_temp_free(bs);
	return true;
}