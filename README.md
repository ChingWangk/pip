# Architecture Labs: Pipeline & ILP – Lab2

### 0. Background

All processors since about 1985 use **pipelining** to overlap the execution of instructions and improve performance. This potential overlap among instructions is called **instruction-level parallelism (ILP)**.

There are two largely separable approaches to exploiting ILP: (1) an approach that relies on hardware to help discover and exploit the parallelism dynamically, and (2) an approach that relies on software technology to find parallelism statically at compile time. The typical representative of the former is **pipelining**. While to achieve the latter, we usually need to perform **ILP analysis** on the program.

### 1. Goal

In lab2, you will finish a **pipeline simulator** using C++. **ARM-like** instructions are read from a file along with their latency. The simulator should visualize the execution of these instructions and output the change of CPU states, as well as the CPI for these instructions.

After that, as an ***optional*** part, you can use QRIO to roughly compare the **ILP** of real-world applications horizontally. If you choose to do this part, you need to use QRIO dependence distance tool to measure the **dependence distance distribution** of SPEC CPU2006 benchmark, and then submit an analysis report.

### 2. Compile and Run

**First, you should update your forked repository, find the new branch "lab2" and change to it**. Based on lab1 (branch "master"), we add a new directory "pipeline_sim", a new markdown file "Lab2.md",  and some new scripts in "cmd" directory. The "pipeline_sim" directory will be your main working directory.

After that, open the terminal, cd to the "cmd" directory, and execute **run-pipeline**. You will find a new file named "result.txt" appears in the "pipeline_sim" directory. It shows which instructions are executed at each stage of the pipeline for each clock cycle, as well as the value of each register at the end of this cycle. For example: 

```bash
# "instruction.txt" is the same content as "testcases/EZCall.txt"

# ...

# In this cycle, pipeline is executing as following

======= WB =======			# Write back the result of a "mov" instr (pc: 0x04)
mov
======= MEM ======			# Access memory for a "mov" instr (pc: 0x08)
mov
======= EX =======			# Execute for a "cmp & bge" instr (pc: 0x0c)
cmp & bge
======= ID =======			# Decode a instr (turn out to be a "bl" instr, pc: 0x10)
bl
======= IF =======			# Fetch a instr (turn out to be a "b" instr, pc: 0x14)
b

# After the above execution, register state is:

reg[0]    = 1         	reg[1]    = 2         	reg[2]    = 0         	reg[3]    = 0         	
reg[4]    = 0         	reg[5]    = 0         	reg[6]    = 0         	reg[7]    = 0         	
reg[8]    = 0         	reg[9]    = 0         	reg[10]   = 0         	reg[11]   = 0         	
reg[12]   = 0         	reg[13]   = 3000      	reg[14]   = 0x0       	reg[15]   = 0x18      	

# Instruction executed so far:
Instr N: 2
# Latency (clock cycle) so far:
Latency: 7

# ...

# CPI = Latency / Instr N
CPI: 3.2
```

***Note***: when you run it for the first time, you will find that the simulator simply fills the instruction sequence into the pipeline in order, and the register state will not change in any way. It's your work to complete the pipeline simulator and make it run correctly.

***Note***: we provide a sample program "pipeline_sim_ref" for reference. We also give 5 sample instruction sequences in "testcases". You can replace the contents of "instruction.txt" with any of these test cases, and then cd to "cmd" and execute **run-pipeline-ref**. You will find a new file named "result_ref.txt" appears in the "pipeline_sim" directory. Observe the test case and the corresponding "result_ref.txt" and you can get more understanding.

### 3. Pipeline Model

The classic pipeline is divided into five stages.

#### IF (Instruction Fetch)

In this stage, you need to fetch the next instruction according to PC. In this project, a **Parser** will read instruction sequences and wrap them into a vector. All you need to do is to get the correct instruction, and then increase PC.

#### ID (Instruction Decode)

For readability, we use the string form of instruction instead of the binary form in "instruction.txt". Thus, in this stage, you needn't actually decode the instruction. All you need to do is to collect the instruction information passed by the **IF** stage.

However, to deal with **data hazards**, you need to support **forwarding**. There are two types of **forwarding**: one is to fetch operand from **EX** stage and the other is from **MEM** stage. (***Note***: to deal with **load-use data hazards**, just forwarding is not enough. Maybe you should do more operations.)

#### EX (Execute)

In this stage, you need to execute according to the operation types and operands given in the **ID** stage. For arithmetic instructions, you need to calculate the result. For memory access instructions, you need to calculate the memory access address. For branch instructions, you need to calculate the target address.

#### MEM (Memory)

In this stage, you should access memory if needed. Also, you need to detect **control hazards** in this stage according to **zero** flag (set in **EX** stage by cmp instructions) and **branch** signal (for simplicity, you only need to check if the **opcode** is a branch instruction).

#### WB (Write Back)

In this stage, you should write calculation results back to registers if needed.

After these five pipeline stages, there remain some hazards (**load-use data hazards** and **control hazards**) to be dealt with (normal **data hazards** have been dealt with in **ID** stage using **forwarding**). Thus, you need to perform some additional operations, such as inserting bubbles, changing the value of PC, etc.

***Note***: there is no branch predictor in this pipeline model. In other words, when encountering branch instructions, you should always treat it as **not taken**, even for branch instructions such as **b** and **bl** (to avoid some unnecessary troubles).

### 4. Assumptions

1. There are 16 32-bit registers: general purpose registers (r0 - r13), stack pointer (sp), link register (lr), and program counter (pc).
2. Total memory space is 6000 bytes. Top address [5001, 6000] is preserved. Stack decreases from 5000, while heap increases from 0.
3. PC starts from 0x0. Memory allocation is defined using ".space" followed by the allocated size (bytes). 
4. Compare (cmp) and the following Branch (bge or bne) are considered to be one instruction, like "cmp reg1, reg2 bge label".
5. Input instruction sequence is taken from "instruction.txt" while latency is taken from "latency.txt".

### 5. TODO

In lab2, your work can be divided into several steps:

1. Familiar with the pipeline simulator source codes in "pipeline_sim/src".
2. Run the reference program, observe the input/output, and understand your goal.
3. Fix and complete each pipeline stage in "pipeline_sim.cpp".
4. Deal with pipeline hazards in "pipeline_sim.cpp".
5. Compare the simulation result of your implementation and the reference program.

    - Directory "testcases" gives 5 test cases, and "pipeline_sim_ref" gives the sample program as a reference. Replace the contents of "instruction.txt" with any of these test cases, and then cd to **cmd** and execute **run-pipeline-ref**, you can get the reference simulation result in "result_ref.txt".
    - ***Given the same input (latency.txt and instruction.txt), your implementation should have the same output as the reference program***.

### 6. Supported Instructions

1. add rd, reg1, reg2
2. add rd, reg1, #imm
3. add sp, sp, #imm
4. sub rd, reg1, reg2
5. sub rd, reg1, #imm
6. sub sp, sp, #imm
7. mul rd, reg1, reg2
8. mul rd, reg1, #imm
9. mov rd, reg1
10. mov rd, #imm
11. mov pc, lr
12. ldr rd, =label
13. ldr rd, [reg1]
14. ldr rd, [reg1, #imm]
15. ldr rd, [sp, #imm]
16. ldr lr, [sp, #imm]
17. str rd, [reg1]
18. str rd, [reg1, #imm]
19. str rd, [sp, #imm]
20. str lr, [sp, #imm]
21. cmp reg1, reg2 bne label
22. cmp reg1, #imm bne label
23. cmp reg1, reg2 bge label
24. cmp reg1, #imm bge label
25. bl procedureLabel
26. b label
27. Exit

### 7. QRIO Dependence Distance Analysis

> **Note**: this part is ***OPTIONAL***

We can use the **dependent distance distribution** of dynamic instruction flow to measure the degree of potential **ILP** of a program. The **dependency distance** is defined as the total number of instructions between the two instructions that writing a register and use the register for the first time. For example, in this instruction sequence:

```
1. mov r0, #0x0				# Write r0
2. mov r1, #0x200			# Write r1
3. ldr r2, [r1, #0x400]		# Read r1, Write r2
4. ldr r3, [r1, #0x404]		# Read r1, Write r3
5. add r0, r0, r2			# Read r0, r2, Write r0
6. sub r0, r0, r3			# Read r0, r3, Write r0
```

**Instruction3** reads r1 for the first time since r1 was written by **instruction2**. Thus, instruction3 generates a dependence distance of 1.

**Instruction5** reads r0 for the first time since r0 was written by **instruction1**, and reads r2 for the first time since r2 was written by **instruction3**. Thus, instruction5 generates two dependence distance: 4 and 2.

...

Instruction-level dependent distance information is useful for understanding the potential **ILP** of a program. Algorithms with a higher percentage of large dependence distances are considered to have a better **ILP**.

You can use QRIO to measure the dependence distance distribution of a program. Just ensure that you are in branch "lab2", and you only need to re-compile and run **QEMU** and **RIO** using the same method in lab1. The result will be dumped to the console, like this:

```
...
Dependence distance tool results:
Threshold config: 0
Total number for distance == 1:        253153 (39%)
Total number for distance <= 2:        325728 (50%)
Total number for distance <= 4:        405312 (62%)
Total number for distance <= 8:        486302 (75%)
Total number for distance <= 16:       568479 (87%)
Total number for distance <= 32:       619373 (95%)
Total number for distance > 32:         30448 (4.7%)
```

You are required to measure the dependence distance distribution of 5 benchmarks in SPEC CPU2006:

- 429.mcf
- 456.hmmer
- 462.libquantum
- 471.omnetpp
- 483.xalancbmk

Then, write an analysis report to compare the **ILP** of these benchmarks.

### 8. Submit

After finishing lab2, please mark your final version using git tag as follows:

```
git tag -a v2.0 -m "Lab2 finish"
```

(We will take the commit marked by this tag as the source code submission of your lab2)

Your submission should contains these files:

- Lab report, which contains:
    - Your lab repository link (formats like https://gitee.com/xxx/qrio_handout.git)
    - Description of your implememtation of each pipeline stage and how you deal with pipeline hazards
    - Comparison of your implementation with the reference program

- (Optional) QRIO dependence distance analysis report, which contains:
    - Explain the relationship between dependence distance distribution and ILP, and why
    - ILP comparison analysis of the 5 SPEC CPU2006 benchmarks mentioned above.

