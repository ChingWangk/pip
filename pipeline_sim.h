#ifndef PIPELINE_SIM_H
#define PIPELINE_SIM_H

#include <iostream>
#include <bits/stdc++.h>
#include <string>

#include "arm_instr.h"

using namespace std;

struct IF_ID {
    int prog_cnt;
    Instruction recent_instr;
};

struct ID_EX {
    int dest;
    int r1;
    int r2;
    int address;
    int offset;

    int prog_cnt;
    ARM_OPC opcode = OPC_INVALID;
    ARM_INSTR_TYPE type = INSTR_TYPE_INVALID;
};

struct EX_MEM {
    int val_arith;
    int val_address;
    int dest;
    bool zero;

    int prog_cnt;
    ARM_OPC opcode = OPC_INVALID;
    ARM_INSTR_TYPE type = INSTR_TYPE_INVALID;
};

struct MEM_WB {
    int val_data;
    int dest;

    int prog_cnt;
    ARM_OPC opcode = OPC_INVALID;
    ARM_INSTR_TYPE type = INSTR_TYPE_INVALID;
};

#endif // PIPELINE_SIM_H
