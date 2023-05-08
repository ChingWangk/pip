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
    int rd;
    int reg1;
    int reg2;
    int val1;
    int val2;
    string label;
    int imm;
    int address;
    int offset;

    int prog_cnt;
    ARM_OPC opcode = OPC_INVALID;
    ARM_INSTR_TYPE type = INSTR_TYPE_INVALID;
    Instruction recent_instr;
};

struct EX_MEM {
    int alu_output;
    int val_address;
    int val1;
    int val2;
    int rd;
    bool zero;

    int prog_cnt;
    ARM_OPC opcode = OPC_INVALID;
    ARM_INSTR_TYPE type = INSTR_TYPE_INVALID;
};

struct MEM_WB {
    int alu_output;
    int rd;

    int prog_cnt;
    ARM_OPC opcode = OPC_INVALID;
    ARM_INSTR_TYPE type = INSTR_TYPE_INVALID;
};

#endif // PIPELINE_SIM_H

