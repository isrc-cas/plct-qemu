#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/main-loop.h"
#include "cpu.h"
#include "exec/exec-all.h"
#include "tcg/tcg-op.h"
#include "trace.h"
#include "exec/memop.h"

#define MAX_COL_NUM 3
static unsigned int matrix_config = MAX_COL_NUM;
static unsigned int row_buffer[MAX_COL_NUM];

void helper_custom_lbuf(CPURISCVState *env, target_ulong rs1)
{
    for (int i = 0; i < matrix_config; i++) {
        row_buffer[i] = cpu_ldl_le_data(env, rs1 +   4 * i );
    }
}

void helper_custom_sbuf(CPURISCVState *env, target_ulong rs1)
{
    for (int i = 0; i < matrix_config; i++) {
        cpu_stl_le_data(env, rs1 + 4 * i, row_buffer[i]);
    }
}

target_ulong helper_custom_rowsum(CPURISCVState *env, target_ulong rs1)
{
    target_ulong rowsum = 0;
    unsigned int data;
    for (int i = 0; i < matrix_config; i++) {
        data = cpu_ldl_le_data(env, rs1 + 4 * i);
        rowsum += data;
        row_buffer[i] += data;
    }
    return rowsum;
}