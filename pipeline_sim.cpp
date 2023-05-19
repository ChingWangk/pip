#include "pipeline_sim.h"
#include "parser.h"

/**
 * For simplicity, we only use the 16 registers in ARM 32.
 * Memory is 4000 bytes, and top 1000 bytes are preserved.
 * Stack pointer (reg[ARM_REG_SP]) is initialized to 3000.
*/
int reg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3000, 0, 0};
int mem[4000 / sizeof(int)];

vector<Instruction> instructions;
vector<int> latencies(12, 0);

bool load_use_hazard = false;
bool control_hazard = false;
bool shut_down = false;
bool file_end = false;

ofstream fout;


// ====================== PIPELINE STAGES ======================

IF_ID Register_IF_ID;
ID_EX Register_ID_EX;
EX_MEM Register_EX_MEM;
MEM_WB Register_MEM_WB;

/**
 * Fetch
*/
void IF() {

    /*************************************/
    /* TODO: Fix and complete IF stage.  */
    /* Hint: if hazards happen, you may  */
    /*  need to change some operations.  */
    /*************************************/

    if ((reg[ARM_REG_PC] / 4) >= instructions.size()) {
        Register_IF_ID.prog_cnt = reg[ARM_REG_PC] / 4;
        Instruction end_file;
        end_file.opcode = END_OF_FILE;
        Register_IF_ID.recent_instr = end_file;
    } else {
        Register_IF_ID.prog_cnt = reg[ARM_REG_PC] / 4;
        Register_IF_ID.recent_instr = instructions[reg[ARM_REG_PC] / 4];
        if (!control_hazard) {
            reg[ARM_REG_PC] += 4;
        }
    }
    fout << ARM_OPC_NAME[Register_IF_ID.recent_instr.opcode] << endl;
}

/**
 * Decode
*/
void ID() {

    /*************************************/
    /* TODO: Fix and complete ID stage.  */
    /* Note: you are expected to detect  */
    /*  load-use data hazard and support */
    /*  forwarding in this stage.        */
    /*************************************/

    Register_ID_EX.prog_cnt = Register_IF_ID.prog_cnt;
    Register_ID_EX.opcode = Register_IF_ID.recent_instr.opcode;
    Register_ID_EX.type = Register_IF_ID.recent_instr.type;
    if (Register_IF_ID.recent_instr.opcode == OPC_LDR && 
    (Register_IF_ID.recent_instr.rd == Register_ID_EX.reg1 || Register_IF_ID.recent_instr.rd == Register_ID_EX.reg2)) {
    load_use_hazard = true;
    }
    int offset = 0;
    if (Register_IF_ID.recent_instr.opcode == OPC_B || Register_IF_ID.recent_instr.opcode == OPC_BL) {
        // 根据指令类型计算偏移量
        switch (Register_IF_ID.recent_instr.type) {
            case OPC_CMPBNE:
            case OPC_CMPBGE:
                offset = Register_IF_ID.recent_instr.imm;
                break;
            default:
                break;
        }
        reg[ARM_REG_PC] += (offset << 2);
        control_hazard = true;
    }
    switch (Register_ID_EX.opcode) {
        case OPC_ADD:
        case OPC_SUB:
        case OPC_MUL:
            Register_ID_EX.rd = Register_IF_ID.recent_instr.rd;
            Register_ID_EX.reg1 = Register_IF_ID.recent_instr.reg1;
            Register_ID_EX.reg2 = Register_IF_ID.recent_instr.reg2;

            if (Register_EX_MEM.rd == Register_ID_EX.reg1) {
                Register_ID_EX.val1 = Register_EX_MEM.alu_output;
            } else if (Register_MEM_WB.rd == Register_ID_EX.reg1) {
                Register_ID_EX.val1 = Register_MEM_WB.alu_output;
            } else {
                Register_ID_EX.val1 = reg[Register_ID_EX.reg1];
            }

            if (Register_EX_MEM.rd == Register_ID_EX.reg2) {
                Register_ID_EX.val2 = Register_EX_MEM.alu_output;
            } else if (Register_MEM_WB.rd == Register_ID_EX.reg2) {
                Register_ID_EX.val2 = Register_MEM_WB.alu_output;
            } else {
                Register_ID_EX.val2 = reg[Register_ID_EX.reg2];
            }

            break;

        case OPC_TYPE_ADD_IMM:
        case OPC_TYPE_SUB_IMM:
        case OPC_TYPE_MUL_IMM:
            Register_ID_EX.rd = Register_IF_ID.recent_instr.rd;
            Register_ID_EX.reg1 = Register_IF_ID.recent_instr.reg1;
            Register_ID_EX.imm = Register_IF_ID.recent_instr.imm;

            if (Register_EX_MEM.rd == Register_ID_EX.reg1) {
                Register_ID_EX.val1 = Register_EX_MEM.alu_output;
            } else if (Register_MEM_WB.rd == Register_ID_EX.reg1) {
                Register_ID_EX.val1 = Register_MEM_WB.alu_output;
            } else {
                Register_ID_EX.val1 = reg[Register_ID_EX.reg1];
            }

            Register_ID_EX.val2 = Register_ID_EX.imm;

            break;

        case OPC_TYPE_ADD_SP_IMM:
        case OPC_TYPE_SUB_SP_IMM:
            Register_ID_EX.rd = Register_IF_ID.recent_instr.rd;
            Register_ID_EX.imm = Register_IF_ID.recent_instr.imm;
            Register_ID_EX.val1 = reg[ARM_REG_SP];
            Register_ID_EX.val2 = Register_ID_EX.imm;

            break;

        case OPC_TYPE_MOV_REG:
            Register_ID_EX.rd = Register_IF_ID.recent_instr.rd;
            Register_ID_EX.reg1 = Register_IF_ID.recent_instr.reg1;

            if (Register_EX_MEM.rd == Register_ID_EX.reg1) {
                Register_ID_EX.val1 = Register_EX_MEM.alu_output;
            } else if (Register_MEM_WB.rd == Register_ID_EX.reg1) {
                Register_ID_EX.val1 = Register_MEM_WB.alu_output;
            } else {
                Register_ID_EX.val1 = reg[Register_ID_EX.reg1];
            }

            break;

        case OPC_TYPE_MOV_IMM:
            Register_ID_EX.rd = Register_IF_ID.recent_instr.rd;
            Register_ID_EX.imm = Register_IF_ID.recent_instr.imm;
            Register_ID_EX.val1 = Register_ID_EX.imm;

            break;
        
        case OPC_TYPE_MOV_PC_LR:
        Register_ID_EX.rd = ARM_REG_PC;
        Register_ID_EX.val1 = reg[ARM_REG_LR];

        break;

    case OPC_TYPE_LDR_LABEL:
    case OPC_TYPE_LDR_REG:
    case OPC_TYPE_LDR_REG_OFFSET:
    case OPC_TYPE_LDR_SP_OFFSET:
    case OPC_TYPE_LDR_LR_SP_OFFSET:
        Register_ID_EX.rd = Register_IF_ID.recent_instr.rd;
        Register_ID_EX.reg1 = Register_IF_ID.recent_instr.reg1;
        Register_ID_EX.imm = Register_IF_ID.recent_instr.imm;

        if (Register_EX_MEM.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_EX_MEM.alu_output;
        } else if (Register_MEM_WB.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_MEM_WB.alu_output;
        } else {
            Register_ID_EX.val1 = reg[Register_ID_EX.reg1];
        }

        break;

    case OPC_TYPE_STR_REG:
    case OPC_TYPE_STR_REG_OFFSET:
    case OPC_TYPE_STR_SP_OFFSET:
    case OPC_TYPE_STR_LR_SP_OFFSET:
        Register_ID_EX.rd = Register_IF_ID.recent_instr.rd;
        Register_ID_EX.reg1 = Register_IF_ID.recent_instr.reg1;
        Register_ID_EX.imm = Register_IF_ID.recent_instr.imm;

        if (Register_EX_MEM.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_EX_MEM.alu_output;
        } else if (Register_MEM_WB.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_MEM_WB.alu_output;
        } else {
            Register_ID_EX.val1 = reg[Register_ID_EX.reg1];
        }

        break;

    case OPC_TYPE_CMP_BNE_REG:
    case OPC_TYPE_CMP_BGE_REG:
    case OPC_TYPE_CMP_BNE_IMM:
    case OPC_TYPE_CMP_BGE_IMM:
        Register_ID_EX.reg1 = Register_IF_ID.recent_instr.reg1;
        Register_ID_EX.reg2 = Register_IF_ID.recent_instr.reg2;
        Register_ID_EX.imm = Register_IF_ID.recent_instr.imm;
        Register_ID_EX.label = Register_IF_ID.recent_instr.label;

        if (Register_EX_MEM.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_EX_MEM.alu_output;
        } else if (Register_MEM_WB.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_MEM_WB.alu_output;
        } else {
            Register_ID_EX.val1 = reg[Register_ID_EX.reg1];
        }

        

        break;

    case OPC_BL:
        Register_ID_EX.label = Register_IF_ID.recent_instr.label;
        reg[ARM_REG_LR] = reg[ARM_REG_PC];

        break;

    case OPC_B:
        Register_ID_EX.label = Register_IF_ID.recent_instr.label;

        break;

    case OPC_EXIT:
        shut_down = true;
        break;

    default:
        break;
    fout << ARM_OPC_NAME[Register_ID_EX.opcode] << endl;
    }
}

/**
 * Execute
*/
void EX() {

    /*************************************/
    /* TODO: Fix and complete EX stage.  */
    /* Hint: finish computing.           */
    /*************************************/

    Register_EX_MEM.prog_cnt = Register_ID_EX.prog_cnt;
    Register_EX_MEM.opcode = Register_ID_EX.opcode;
    Register_EX_MEM.type = Register_ID_EX.type;
    Register_EX_MEM.rd = Register_ID_EX.rd;
Register_EX_MEM.val1 = Register_ID_EX.val1;
Register_EX_MEM.val2 = Register_ID_EX.val2;
Register_EX_MEM.val_address = Register_ID_EX.address;

switch (Register_EX_MEM.opcode) {
    case OPC_ADD:
    case OPC_TYPE_ADD_IMM:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1 + Register_EX_MEM.val2;
        break;

    case OPC_SUB:
    case OPC_TYPE_SUB_IMM:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1 - Register_EX_MEM.val2;
        break;

    case OPC_MUL:
    case OPC_TYPE_MUL_IMM:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1 * Register_EX_MEM.val2;
        break;

    case OPC_TYPE_ADD_SP_IMM:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1 + Register_EX_MEM.val2;
        break;

    case OPC_TYPE_SUB_SP_IMM:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1 - Register_EX_MEM.val2;
        break;

    case OPC_TYPE_MOV_REG:
    case OPC_TYPE_MOV_IMM:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1;
        break;

    case OPC_TYPE_MOV_PC_LR:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1;
        break;

    case OPC_TYPE_LDR_LABEL:
    case OPC_TYPE_LDR_REG:
    case OPC_TYPE_LDR_REG_OFFSET:
    case OPC_TYPE_LDR_SP_OFFSET:
    case OPC_TYPE_LDR_LR_SP_OFFSET:
        Register_EX_MEM.val_address = Register_EX_MEM.val1 + Register_EX_MEM.val2;
        break;

    case OPC_TYPE_STR_REG:
    case OPC_TYPE_STR_REG_OFFSET:
    case OPC_TYPE_STR_SP_OFFSET:
    case OPC_TYPE_STR_LR_SP_OFFSET:
        Register_EX_MEM.val_address = Register_EX_MEM.val1 + Register_EX_MEM.val2;
        break;

    case OPC_TYPE_CMP_BNE_REG:
    case OPC_TYPE_CMP_BGE_REG:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1 - Register_EX_MEM.val2;
        break;

    case OPC_TYPE_CMP_BNE_IMM:
    case OPC_TYPE_CMP_BGE_IMM:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1 - Register_EX_MEM.val2;
        break;

    case OPC_BL:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1;
        break;

    case OPC_B:
        Register_EX_MEM.alu_output = Register_EX_MEM.val1;
        break;

    case OPC_EXIT:
        break;

    default:
        break;
}
    fout << ARM_OPC_NAME[Register_EX_MEM.opcode] << endl;
}

/**
 * Memory
*/
void MEM() {

    /*************************************/
    /* TODO: Fix and complete MEM stage. */
    /* Note: you are expected to detect  */
    /*  control hazards in this stage.   */
    /*************************************/

    if (Register_EX_MEM.rd != ARM_REG_INVALID) {
        if (Register_EX_MEM.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_EX_MEM.alu_output;
        }
        if (Register_EX_MEM.rd == Register_ID_EX.reg2) {
            Register_ID_EX.val2 = Register_EX_MEM.alu_output;
        }
    }
    if (Register_MEM_WB.rd != ARM_REG_INVALID) {
        if (Register_MEM_WB.rd == Register_ID_EX.reg1) {
            Register_ID_EX.val1 = Register_MEM_WB.alu_output;
        }
        if (Register_MEM_WB.rd == Register_ID_EX.reg2) {
            Register_ID_EX.val2 = Register_MEM_WB.alu_output;
        }
    }

    switch (Register_EX_MEM.opcode) {
        case OPC_ADD:
        case OPC_SUB:
        case OPC_MUL:
        case OPC_MOV:
            Register_MEM_WB.alu_output = Register_EX_MEM.alu_output;
            Register_MEM_WB.rd = Register_EX_MEM.rd;
            Register_MEM_WB.prog_cnt = Register_EX_MEM.prog_cnt;
            Register_MEM_WB.opcode = Register_EX_MEM.opcode;
            Register_MEM_WB.type = Register_EX_MEM.type;
            break;

        case OPC_TYPE_LDR_LABEL:
        case OPC_TYPE_LDR_REG:
        case OPC_TYPE_LDR_REG_OFFSET:
        case OPC_TYPE_LDR_SP_OFFSET:
        case OPC_TYPE_LDR_LR_SP_OFFSET:
            Register_MEM_WB.alu_output = mem[Register_EX_MEM.val_address / 4];
            Register_MEM_WB.rd = Register_EX_MEM.rd;
            Register_MEM_WB.prog_cnt = Register_EX_MEM.prog_cnt;
            Register_MEM_WB.opcode = Register_EX_MEM.opcode;
            Register_MEM_WB.type = Register_EX_MEM.type;
            break;

        case OPC_TYPE_STR_REG:
        case OPC_TYPE_STR_REG_OFFSET:
        case OPC_TYPE_STR_SP_OFFSET:
        case OPC_TYPE_STR_LR_SP_OFFSET:
            mem[Register_EX_MEM.val_address / 4] = reg[Register_EX_MEM.rd];
            break;

        case OPC_CMPBNE:
        case OPC_CMPBGE:
            break;

        case OPC_BL:
        case OPC_B:
            Register_MEM_WB.prog_cnt = Register_EX_MEM.prog_cnt;
            Register_MEM_WB.opcode = Register_EX_MEM.opcode;
            Register_MEM_WB.type = Register_EX_MEM.type;
            break;

        case OPC_EXIT:
            shut_down = true;
            break;

        default:
            break;
    }
    fout << ARM_OPC_NAME[Register_EX_MEM.opcode] << endl;
}
    


/**
 * Write Back
*/
void WB() {

    /*************************************/
    /* TODO: Fix and complete WB stage.  */
    /* Hint: update the register state.  */
    /*************************************/

    switch (Register_MEM_WB.opcode) {
    case OPC_ADD:
    case OPC_SUB:
    case OPC_MUL:
        reg[Register_MEM_WB.rd] = Register_MEM_WB.alu_output;
        break;
    case OPC_MOV:
        reg[Register_MEM_WB.rd] = Register_MEM_WB.alu_output;
        break;
    case OPC_TYPE_LDR_LABEL:
    case OPC_TYPE_LDR_REG:
    case OPC_TYPE_LDR_REG_OFFSET:
    case OPC_TYPE_LDR_SP_OFFSET:
    case OPC_TYPE_LDR_LR_SP_OFFSET:
        reg[Register_MEM_WB.rd] = Register_MEM_WB.alu_output;
        break;
    case OPC_CMPBNE:
    case OPC_CMPBGE:
        break;
    case OPC_BL:
        reg[ARM_REG_LR] = Register_MEM_WB.prog_cnt * 4 + 4;
        reg[ARM_REG_PC] = (Register_MEM_WB.alu_output / 4) * 4;
        break;
    case OPC_B:
        reg[ARM_REG_PC] = (Register_MEM_WB.alu_output / 4) * 4;
        break;
    case OPC_EXIT:
        shut_down = true;
        break;
    case END_OF_FILE:
        file_end = true;
        break;
    case BUBBLE:
        break;
    default:
        break;
}

if (load_use_hazard) {
    Register_ID_EX.recent_instr.opcode = OPC_INVALID;
    Register_ID_EX.recent_instr.type = INSTR_TYPE_INVALID;
    Register_ID_EX.recent_instr.rd = -1;
    Register_ID_EX.recent_instr.reg1 = -1;
    Register_ID_EX.recent_instr.reg2 = -1;
    Register_ID_EX.type = INSTR_TYPE_INVALID;
    Register_ID_EX.opcode = OPC_INVALID;
    Register_ID_EX.rd = -1;
    Register_ID_EX.reg1 = -1;
    Register_ID_EX.reg2 = -1;
    Register_ID_EX.address = -1;
    Register_ID_EX.offset = -1;
    load_use_hazard = false;
}

if (control_hazard) {
    Register_ID_EX.recent_instr.opcode = OPC_INVALID;
    Register_ID_EX.recent_instr.type = INSTR_TYPE_INVALID;
    Register_ID_EX.recent_instr.rd = -1;
    Register_ID_EX.recent_instr.reg1 = -1;
    Register_ID_EX.recent_instr.reg2 = -1;
    Register_IF_ID.prog_cnt = Register_EX_MEM.alu_output / 4;
    control_hazard = false;
}

fout << ARM_OPC_NAME[Register_MEM_WB.opcode] << endl;
}

// ====================== PIPELINE STAGES ======================


void deal_with_hazards() {
    if (load_use_hazard) {
        if (Register_MEM_WB.type == INSTR_TYPE_REG && (Register_MEM_WB.rd == Register_ID_EX.reg1 || Register_MEM_WB.rd == Register_ID_EX.reg2)) {
            IF_ID temp_if_id;
            temp_if_id.prog_cnt = Register_ID_EX.prog_cnt;
            temp_if_id.recent_instr.opcode = BUBBLE;
            Register_IF_ID = temp_if_id;
            load_use_hazard = false;
        }
        /*******************************************************************/
        /* TODO: additional operations to deal with load-use data hazards. */
        /* Hint: inserting a bubble between load and use.                  */
        /*******************************************************************/

    }
    if (control_hazard) {
        reg[ARM_REG_PC] = Register_ID_EX.prog_cnt * 4;
        IF_ID temp_if_id;
        temp_if_id.prog_cnt = Register_ID_EX.prog_cnt;
        temp_if_id.recent_instr.opcode = BUBBLE;
        Register_IF_ID = temp_if_id;
        ID_EX temp_id_ex;
        temp_id_ex.prog_cnt = -1;
        Register_ID_EX = temp_id_ex;
        EX_MEM temp_ex_mem;
        temp_ex_mem.prog_cnt = -1;
        Register_EX_MEM = temp_ex_mem;
        control_hazard = false;
    }
        /*******************************************************************/
        /* TODO: additional operations to deal with control hazards.       */
        /* Hint: roll back IF & ID & EX stages; reset PC to correct value. */
        /*******************************************************************/

}

void print_register() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int num = 4 * i + j;
            char reg_str[10], eq_str[10];
            sprintf(reg_str, "reg[%d]", num);
            if (num < ARM_REG_LR) {
                sprintf(eq_str, "= %d", reg[num]);
            } else {
                sprintf(eq_str, "= 0x%x", reg[num]);
            }
            fout << left << setw(10) << reg_str << setw(12) << eq_str << "\t";
        }
        fout << endl;
    }
    fout << endl;
}

int compute_latency() {
    vector<int> latency_pipe(5, 0);
    // IF
    latency_pipe[0] = 1;
    // ID
    latency_pipe[1] = 1;
    // EX
    if (Register_ID_EX.opcode > OPC_INVALID && Register_ID_EX.opcode <= OPC_MOV) {
        latency_pipe[2] = latencies[Register_ID_EX.opcode - 1];
    } else if (Register_ID_EX.opcode == OPC_LDR && Register_ID_EX.type == INSTR_TYPE_EXTRA) {
        latency_pipe[2] = latencies[5];
    } else if (Register_ID_EX.opcode >= OPC_CMPBNE && Register_ID_EX.opcode <= OPC_CMPBGE) {
        latency_pipe[2] = latencies[7];
    } else {
        latency_pipe[2] = 1;
    }
    // MEM
    if (Register_EX_MEM.opcode == OPC_LDR) {
        latency_pipe[3] = latencies[4];
    } else if (Register_EX_MEM.opcode == OPC_STR) {
        latency_pipe[3] = latencies[6];
    } else if (Register_EX_MEM.opcode >= OPC_CMPBNE && Register_EX_MEM.opcode <= OPC_B) {
        latency_pipe[3] = latencies[Register_EX_MEM.opcode + 1];
    } else {
        latency_pipe[3] = 1;
    }
    // WB
    latency_pipe[4] = 1;
    return *max_element(latency_pipe.begin(), latency_pipe.end());
}

int main() {

    memset(reg, 0, sizeof(reg));
    memset(mem, 0, sizeof(mem));
    if (parse_file(latencies, instructions)) {
        return -1;
    }

    int instr_num = 0, latency = 0;

    fout.open("result.txt");
    print_register();

    while (!shut_down && !file_end) {
        if (Register_MEM_WB.opcode >= OPC_ADD && Register_MEM_WB.opcode <= OPC_B) {
            instr_num++;
        }

        fout << "======= WB =======" << endl;   WB();
        fout << "======= MEM ======" << endl;   MEM();
        fout << "======= EX =======" << endl;   EX();
        fout << "======= ID =======" << endl;   ID();
        fout << "======= IF =======" << endl;   IF();

        deal_with_hazards();

        fout << endl;
        print_register();

        latency += compute_latency();

        fout << "Instr N: " << instr_num << endl;
        fout << "Latency: " << latency << endl << endl << endl;
    }

    fout << "CPI: " << float(latency) / float(instr_num) << endl;
    fout.close();

}
