/*
 * QEMU MOS65XX CPU
 *
 * Copyright (c) 2022 Alexandre Guyon
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/lgpl-2.1.html>
 */

#include "qemu/osdep.h"
#include "tcg/tcg.h"
#include "tcg/tcg-op.h"
#include "cpu.h"
#include "exec/exec-all.h"
#include "exec/translator.h"
#include "disas/disas.h"
#include "exec/log.h"

static TCGv cpu_pc;
static TCGv cpu_sr;
static TCGv cpu_sp;
static TCGv cpu_x;
static TCGv cpu_y;
static TCGv cpu_acc;

typedef struct DisasContext DisasContext;

struct DisasContext {
    DisasContextBase base;

    CPUMOS65XXState *env;
    CPUState *cs;
};

void mos65xx_cpu_tcg_init(void)
{
#define MOS65XX_REG_OFFS(x) offsetof(CPUMOS65XXState, x)
#define MOS65XX_TCG_CPU(TCGv, envName, niceName) \
    TCGv = tcg_global_mem_new_i32(cpu_env, MOS65XX_REG_OFFS(envName), niceName);

    MOS65XX_TCG_CPU(cpu_pc, pc, "pc")
    MOS65XX_TCG_CPU(cpu_sr, sr, "sr")
    MOS65XX_TCG_CPU(cpu_sp, sp, "sp")
    MOS65XX_TCG_CPU(cpu_x, x, "x")
    MOS65XX_TCG_CPU(cpu_y, y, "y")
    MOS65XX_TCG_CPU(cpu_acc, acc, "acc")

#undef MOS65XX_TCG_CPU
#undef MOS65XX_REG_OFFS
}

#define DISAS_EXIT   DISAS_TARGET_0  /* We want return to the cpu main loop.  */
#define QEMU_EXIT    DISAS_TARGET_1  /* We want return to the cpu main loop.  */

#include "exec/gen-icount.h"

static uint32_t decode_insn_load_bytes(DisasContext *ctx, uint32_t insn,
                           int i, int n)
{
    while (++i <= n) {
        uint8_t b = cpu_ldub_code(ctx->env, ctx->base.pc_next++);
        insn |= b << (32 - i * 8);
    }
    return insn;
}

bool decode_insn(DisasContext *ctx, uint32_t insn);
uint32_t decode_insn_load(DisasContext *ctx);
#include "decode-insn.c.inc"

typedef void (*trans_Func)(DisasContext *ctx, TCGv, TCGv);

static void zpg_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv addr = tcg_temp_local_new();
    tcg_gen_movi_tl(addr, value);

    func(ctx, reg, addr);

    tcg_temp_free(addr);
}

static void zpg_x_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv xAddr = tcg_temp_local_new();
    TCGv addr = tcg_temp_local_new();

    tcg_gen_movi_tl(addr, value);
    tcg_gen_add_tl(xAddr, cpu_x, addr);

    func(ctx, reg, addr);

    tcg_temp_free(addr);
    tcg_temp_free(xAddr);
}

static void zpg_y_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv xAddr = tcg_temp_local_new();
    TCGv addr = tcg_temp_local_new();

    tcg_gen_movi_tl(addr, value);
    tcg_gen_add_tl(xAddr, cpu_y, addr);

    func(ctx, reg, addr);

    tcg_temp_free(addr);
    tcg_temp_free(xAddr);
}

static void abs_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv addr = tcg_temp_local_new();
    tcg_gen_movi_tl(addr, value);

    func(ctx, reg, addr);

    tcg_temp_free(addr);
}

static void abs_x_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv xAddr = tcg_temp_local_new();
    TCGv addr = tcg_temp_local_new();

    tcg_gen_movi_tl(addr, value);
    tcg_gen_add_tl(xAddr, cpu_x, addr);

    func(ctx, reg, addr);

    tcg_temp_free(addr);
    tcg_temp_free(xAddr);
}

static void abs_y_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv xAddr = tcg_temp_local_new();
    TCGv addr = tcg_temp_local_new();

    tcg_gen_movi_tl(addr, value);
    tcg_gen_add_tl(xAddr, cpu_y, addr);

    func(ctx, reg, addr);

    tcg_temp_free(addr);
    tcg_temp_free(xAddr);
}

static void ind_y_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv xAddr = tcg_temp_local_new();
    TCGv pAddr = tcg_temp_local_new();
    TCGv addr = tcg_temp_local_new();

    tcg_gen_movi_tl(pAddr, value);
    tcg_gen_qemu_ld16u(addr, pAddr, 0); // FIXME (alnikk) endianness?

    tcg_gen_add_tl(xAddr, cpu_y, addr);

    func(ctx, reg, addr);

    tcg_temp_free(addr);
    tcg_temp_free(pAddr);
    tcg_temp_free(xAddr);
}

static void ind_x_addressing(DisasContext *ctx, trans_Func func, TCGv reg, uint32_t value)
{
    TCGv xAddr = tcg_temp_local_new();
    TCGv pAddr = tcg_temp_local_new();

    tcg_gen_movi_tl(pAddr, value);
    tcg_gen_add_tl(xAddr, cpu_x, pAddr);

    func(ctx, reg, xAddr);

    tcg_temp_free(pAddr);
    tcg_temp_free(xAddr);
}

static void ld(DisasContext *ctx, TCGv reg, TCGv addr)
{
    tcg_gen_qemu_ld8u(reg, addr, 0);
    // TODO (alnikk) update status reg
}

static void st(DisasContext *ctx, TCGv reg, TCGv addr)
{
    tcg_gen_qemu_st8(cpu_acc, addr, 0);
}

static bool trans_LDA_IMM(DisasContext *ctx, arg_LDA_IMM *a)
{
    tcg_gen_movi_tl(cpu_acc, a->imm_p);

    return true;
}

static bool trans_LDA_ZPG(DisasContext *ctx, arg_LDA_ZPG *a)
{
    zpg_addressing(ctx, ld, cpu_acc, a->zpg_p);

    return true;
}

static bool trans_LDA_ZPG_X(DisasContext *ctx, arg_LDA_ZPG_X *a)
{
    zpg_x_addressing(ctx, ld, cpu_acc, a->zpg_p);

    return true;
}

static bool trans_LDA_ABS(DisasContext *ctx, arg_LDA_ABS *a)
{
    abs_addressing(ctx, ld, cpu_acc, a->abs_p);

    return true;
}

static bool trans_LDA_ABS_X(DisasContext *ctx, arg_LDA_ABS_X *a)
{
    abs_x_addressing(ctx, ld, cpu_acc, a->abs_p);

    return true;
}

static bool trans_LDA_ABS_Y(DisasContext *ctx, arg_LDA_ABS_Y *a)
{
    abs_y_addressing(ctx, ld, cpu_acc, a->abs_p);

    return true;
}

static bool trans_LDA_IND_Y(DisasContext *ctx, arg_LDA_IND_Y *a)
{
    ind_y_addressing(ctx, ld, cpu_acc, a->ind_p);

    return true;
}

static bool trans_LDA_X_IND(DisasContext *ctx, arg_LDA_X_IND *a)
{
    ind_x_addressing(ctx, ld, cpu_acc, a->ind_p);

    return true;
}

static bool trans_LDX_IMM(DisasContext *ctx, arg_LDX_IMM *a)
{
    tcg_gen_movi_tl(cpu_x, a->imm_p);

    return true;
}


static bool trans_LDX_ZPG(DisasContext *ctx, arg_LDX_ZPG *a)
{
    zpg_addressing(ctx, ld, cpu_x, a->zpg_p);

    return true;
}

static bool trans_LDX_ZPG_Y(DisasContext *ctx, arg_LDX_ZPG_Y *a)
{
    zpg_y_addressing(ctx, ld, cpu_x, a->zpg_p);

    return true;
}

static bool trans_LDX_ABS(DisasContext *ctx, arg_LDX_ABS *a)
{
    abs_addressing(ctx, ld, cpu_x, a->abs_p);

    return true;
}

static bool trans_LDX_ABS_Y(DisasContext *ctx, arg_LDX_ABS_Y *a)
{
    abs_y_addressing(ctx, ld, cpu_x, a->abs_p);

    return true;
}

static bool trans_LDY_IMM(DisasContext *ctx, arg_LDY_IMM *a)
{
    tcg_gen_movi_tl(cpu_y, a->imm_p);

    return true;
}

static bool trans_LDY_ZPG(DisasContext *ctx, arg_LDY_ZPG *a)
{
    zpg_addressing(ctx, ld, cpu_y, a->zpg_p);

    return true;
}

static bool trans_LDY_ZPG_X(DisasContext *ctx, arg_LDY_ZPG_X *a)
{
    zpg_x_addressing(ctx, ld, cpu_y, a->zpg_p);

    return true;
}

static bool trans_LDY_ABS(DisasContext *ctx, arg_LDY_ABS *a)
{
    abs_addressing(ctx, ld, cpu_y, a->abs_p);

    return true;
}

static bool trans_LDY_ABS_X(DisasContext *ctx, arg_LDY_ABS_X *a)
{
    abs_x_addressing(ctx, ld, cpu_y, a->abs_p);

    return true;
}

static bool trans_STA_ZPG(DisasContext *ctx, arg_STA_ZPG *a)
{
    zpg_addressing(ctx, st, cpu_acc, a->zpg_p);

    return true;
}

static bool trans_STA_ZPG_X(DisasContext *ctx, arg_STA_ZPG_X *a)
{
    zpg_x_addressing(ctx, st, cpu_acc, a->zpg_p);

    return true;
}

static bool trans_STA_ABS(DisasContext *ctx, arg_STA_ABS *a)
{
    abs_addressing(ctx, st, cpu_acc, a->abs_p);

    return true;
}

static bool trans_STA_ABS_X(DisasContext *ctx, arg_STA_ABS_X *a)
{
    abs_x_addressing(ctx, st, cpu_acc, a->abs_p);

    return true;
}

static bool trans_STA_ABS_Y(DisasContext *ctx, arg_STA_ABS_Y *a)
{
    abs_y_addressing(ctx, st, cpu_acc, a->abs_p);

    return true;
}

static bool trans_STA_X_IND(DisasContext *ctx, arg_STA_X_IND *a)
{
    ind_x_addressing(ctx, st, cpu_acc, a->ind_p);

    return true;
}

static bool trans_STA_IND_Y(DisasContext *ctx, arg_STA_IND_Y *a)
{
    ind_y_addressing(ctx, st, cpu_acc, a->ind_p);

    return true;
}

static bool trans_STX_ZPG(DisasContext *ctx, arg_STX_ZPG *a)
{
    zpg_addressing(ctx, st, cpu_x, a->zpg_p);

    return true;
}

static bool trans_STX_ZPG_Y(DisasContext *ctx, arg_STX_ZPG_Y *a)
{
    zpg_y_addressing(ctx, st, cpu_x, a->zpg_p);

    return true;
}

static bool trans_STX_ABS(DisasContext *ctx, arg_STX_ABS *a)
{
    abs_addressing(ctx, st, cpu_x, a->abs_p);

    return true;
}

static bool trans_STY_ZPG(DisasContext *ctx, arg_STY_ZPG *a)
{
    zpg_addressing(ctx, st, cpu_y, a->zpg_p);

    return true;
}

static bool trans_STY_ZPG_X(DisasContext *ctx, arg_STY_ZPG_X *a)
{
    zpg_x_addressing(ctx, st, cpu_y, a->zpg_p);

    return true;
}

static bool trans_STY_ABS(DisasContext *ctx, arg_STY_ABS *a)
{
    abs_addressing(ctx, st, cpu_y, a->abs_p);

    return true;
}

static bool trans_TAX(DisasContext *ctx, arg_TAX *a)
{
    tcg_gen_qemu_ld8u(cpu_x, cpu_acc, 0);

    return true;
}

static bool trans_TAY(DisasContext *ctx, arg_TAY *a)
{
    tcg_gen_qemu_ld8u(cpu_y, cpu_acc, 0);

    return true;
}

static bool trans_TXA(DisasContext *ctx, arg_TXA *a)
{
    tcg_gen_qemu_ld8u(cpu_acc, cpu_x, 0);

    return true;
}

static bool trans_TYA(DisasContext *ctx, arg_TXA *a)
{
    tcg_gen_qemu_ld8u(cpu_acc, cpu_y, 0);

    return true;
}

static bool trans_BREAK(DisasContext *ctx, arg_BREAK *a)
{
    /* NOP */
#define QUIT_ON_BREAK
#if defined(EXCP_DEBUG_ON_BREAK)

    ctx->base.is_jmp = DISAS_EXIT;
    gen_helper_break(cpu_env);

#elif defined(QUIT_ON_BREAK)

    ctx->base.is_jmp = QEMU_EXIT;
    gen_helper_quit(cpu_env);

#endif

    return true;
}

static void mos65xx_tr_init_disas_context(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);
    CPUMOS65XXState *env = cs->env_ptr;

    ctx->cs = cs;
    ctx->env = env;
}

static void mos65xx_tr_tb_start(DisasContextBase *db, CPUState *cs)
{
    if (db->pc_first == PC_LAST_INSN) {
        qemu_log("asking to translate over the end of the address space (pc = 0x%x)\n", db->pc_first);
        gen_helper_quit(cpu_env);
    }
}

static void mos65xx_tr_insn_start(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    tcg_gen_insn_start(ctx->env->pc);
}


static void mos65xx_tr_translate_insn(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    uint32_t insn = decode_insn_load(ctx);
    if (!decode_insn(ctx, insn)) {
        error_report("illegal instruction %08x", insn);
        exit(0);
    }

    if (ctx->base.is_jmp == DISAS_NEXT && ctx->base.pc_next == PC_LAST_INSN) { // There's 3 interrupts byte at the end
        ctx->base.is_jmp = DISAS_TOO_MANY;
    }
}

static void mos65xx_tr_tb_stop(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    switch (ctx->base.is_jmp) {
    case DISAS_NORETURN:
        break;
    case DISAS_NEXT:
    case DISAS_TOO_MANY:
        tcg_gen_movi_i32(cpu_pc, ctx->base.pc_next);
        tcg_gen_lookup_and_goto_ptr();
        ctx->base.is_jmp = DISAS_NORETURN;
        break;
    case DISAS_EXIT:
        tcg_gen_movi_i32(cpu_pc, ctx->base.pc_next);
        tcg_gen_exit_tb(NULL, 0);
        /* fall through */
    case QEMU_EXIT:
        ctx->base.is_jmp = DISAS_NORETURN;
        break;
    default:
        g_assert_not_reached();
    }
}

static void mos65xx_tr_disas_log(const DisasContextBase *dcbase, CPUState *cs)
{
    qemu_log("IN: %s\n", lookup_symbol(dcbase->pc_first));
    log_target_disas(cs, dcbase->pc_first, dcbase->tb->size);
}

static const TranslatorOps mos65xx_tr_ops = {
    .init_disas_context = mos65xx_tr_init_disas_context,
    .tb_start           = mos65xx_tr_tb_start,
    .insn_start         = mos65xx_tr_insn_start,
    .translate_insn     = mos65xx_tr_translate_insn,
    .tb_stop            = mos65xx_tr_tb_stop,
    .disas_log          = mos65xx_tr_disas_log,
};

#define MAX_INSNS TARGET_PAGE_SIZE
void gen_intermediate_code(CPUState *cs, TranslationBlock *tb, int max_insns)
{
    DisasContext dc = { };
    translator_loop(&mos65xx_tr_ops, &dc.base, cs, tb, max_insns > MAX_INSNS ? MAX_INSNS : max_insns);
}

void restore_state_to_opc(CPUMOS65XXState *env, TranslationBlock *tb,
                            target_ulong *data)
{
    env->pc = data[0];
}
