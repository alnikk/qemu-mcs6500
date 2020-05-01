#include "qemu/osdep.h"
#include "qemu/log.h"
#include "cpu.h"
#include "exec/exec-all.h"

#include "exec/translator.h"

typedef struct DisasContext {
    DisasContextBase base;
} DisasContext;

static void mcs6500_tr_init_disas_context(DisasContextBase *dcbase, CPUState *cs)
{
}

static void mcs6500_tr_tb_start(DisasContextBase *db, CPUState *cpu)
{
}

static void mcs6500_tr_insn_start(DisasContextBase *dcbase, CPUState *cpu)
{
}

static bool mcs6500_tr_breakpoint_check(DisasContextBase *dcbase, CPUState *cpu,
                                      const CPUBreakpoint *bp)
{
    return false;
}

static void mcs6500_tr_translate_insn(DisasContextBase *dcbase, CPUState *cpu)
{
}

static void mcs6500_tr_tb_stop(DisasContextBase *dcbase, CPUState *cpu)
{
}

static void mcs6500_tr_disas_log(const DisasContextBase *dcbase, CPUState *cpu)
{
}

static const TranslatorOps mcs6500_tr_ops = {
    .init_disas_context = mcs6500_tr_init_disas_context,
    .tb_start           = mcs6500_tr_tb_start,
    .insn_start         = mcs6500_tr_insn_start,
    .breakpoint_check   = mcs6500_tr_breakpoint_check,
    .translate_insn     = mcs6500_tr_translate_insn,
    .tb_stop            = mcs6500_tr_tb_stop,
    .disas_log          = mcs6500_tr_disas_log,
};

void gen_intermediate_code(CPUState *cs, TranslationBlock *tb, int max_insns)
{
    DisasContext ctx;

    translator_loop(&mcs6500_tr_ops, &ctx.base, cs, tb, max_insns);
}

void restore_state_to_opc(CPUMCS6500State *env, TranslationBlock *tb,
                          target_ulong *data)
{
    env->pc = (uint16_t) (data[0] & PC_MASK);
}
