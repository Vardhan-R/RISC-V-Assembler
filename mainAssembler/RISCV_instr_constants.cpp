#include<iostream>
#include<bits/stdc++.h>
#include "RISCV_instr_constants.h"
using namespace std;


unordered_map<string, int> inst_type = {
    {"add",R},
    {"sub",R},
    {"sll",R},
    {"slt",R},
    {"sltu",R},
    {"xor",R},
    {"srl",R},
    {"sra",R},
    {"or",R},
    {"and",R},


    {"addi",I},
    {"slti",I},
    {"sltiu",I},
    {"xori",I},
    {"ori",I},
    {"andi",I},
    {"slli",I},
    {"srli",I},
    {"srai",I},

    {"lb",I},
    {"lh",I},
    {"lw",I},
    {"ld",I},
    {"lbu",I},
    {"lhu",I},
    {"lwu",I},

    {"sb",S},
    {"sh",S},
    {"sw",S},
    {"sd",S},

    {"beq",B},
    {"bne",B},
    {"blt",B},
    {"bge",B},
    {"bltu",B},
    {"bgeu",B},

    {"lui",U},
    {"auipc",U},

    {"jal",J},
    {"jalr",I},

    {"ecall",I},
    {"ebreak",I}
};

unordered_map<string, int> alias_to_ind = {
        {"zero", 0},
        {"ra", 1},
        {"sp", 2},
        {"gp", 3},
        {"tp", 4},
        {"t0", 5},
        {"t1", 6},
        {"t2", 7},
        {"s0", 8},
        {"fp", 8},
        {"s1", 9},
        {"a0", 10},
        {"a1", 11},
        {"a2", 12},
        {"a3", 13},
        {"a4", 14},
        {"a5", 15},
        {"a6", 16},
        {"a7", 17},
        {"s2", 18},
        {"s3", 19},
        {"s4", 20},
        {"s5", 21},
        {"s6", 22},
        {"s7", 23},
        {"s8", 24},
        {"s9", 25},
        {"s10", 26},
        {"s11", 27},
        {"t3", 28},
        {"t4", 29},
        {"t5", 30},
        {"t6", 31}
};

unordered_map<string, int> funct7_table = {
        {"add", 0x00},
        {"sub", 0x20},
        {"xor", 0x00},
        {"or", 0x00},
        {"and", 0x00},
        {"sll", 0x00},
        {"srl", 0x00},
        {"sra", 0x20},
        {"slt", 0x00},
        {"sltu", 0x00}
};

unordered_map<string, int> funct3_table = {
        {"add", 0x0},
        {"sub", 0x0},
        {"xor", 0x4},
        {"or", 0x6},
        {"and", 0x7},
        {"sll", 0x1},
        {"srl", 0x5},
        {"sra", 0x5},
        {"slt", 0x2},
        {"sltu", 0x3},

        {"addi", 0x0},
        {"xori", 0x4},
        {"ori", 0x6},
        {"andi", 0x7},
        {"slli", 0x1},
        {"srli", 0x5},
        {"srai", 0x5},
        {"slti", 0x2},
        {"sltiu", 0x3},

        {"lb", 0x0},
        {"lh", 0x1},
        {"lw", 0x2},
        {"ld", 0x3},
        {"lbu", 0x4},
        {"lhu", 0x5},
        {"lwu", 0x6},

        {"sb", 0x0},
        {"sh", 0x1},
        {"sw", 0x2},
        {"sd", 0x3},

        {"beq", 0x0},
        {"bne", 0x1},
        {"blt", 0x4},
        {"bge", 0x5},
        {"bltu", 0x6},
        {"bgeu", 0x7},

        {"jalr", 0x0},

        {"ecall", 0x0},

        {"ebreak", 0x0}
};

unordered_map<string, int> funct6_table = {
        {"addi", -1},
        {"xori", -1},
        {"ori", -1},
        {"andi", -1},
        {"slli", 0x00},
        {"srli", 0x00},
        {"srai", 0x10},
        {"slti", -1},
        {"sltiu", -1},
        {"lb", -1},
        {"lh", -1},
        {"lw", -1},
        {"ld", -1},
        {"lbu", -1},
        {"lhu", -1},
        {"lwu", -1},
        {"jalr", -1},
        {"ecall", 0x0},
        {"ebreak", 0x1}
};

unordered_map<string, int> opcode_table = {
        {"add", 0b0110011},
        {"sub", 0b0110011},
        {"xor", 0b0110011},
        {"or", 0b0110011},
        {"and", 0b0110011},
        {"sll", 0b0110011},
        {"srl", 0b0110011},
        {"sra", 0b0110011},
        {"slt", 0b0110011},
        {"sltu", 0b0110011},

        {"addi", 0b0010011},
        {"xori", 0b0010011},
        {"ori", 0b0010011},
        {"andi", 0b0010011},
        {"slli", 0b0010011},
        {"srli", 0b0010011},
        {"srai", 0b0010011},
        {"slti", 0b0010011},
        {"sltiu", 0b0010011},

        {"lb", 0b0000011},
        {"lh", 0b0000011},
        {"lw", 0b0000011},
        {"ld", 0b0000011},
        {"lbu", 0b0000011},
        {"lhu", 0b0000011},
        {"lwu", 0b0000011},

        {"sb", 0b0100011},
        {"sh", 0b0100011},
        {"sw", 0b0100011},
        {"sd", 0b0100011},

        {"beq", 0b1100011},
        {"bne", 0b1100011},
        {"blt", 0b1100011},
        {"bge", 0b1100011},
        {"bltu", 0b1100011},
        {"bgeu", 0b1100011},

        {"jal", 0b1101111},
        {"jalr", 0b1100111},

        {"lui", 0b0110111},
        {"auipc", 0b0010111},

        {"ecall", 0b1110011},

        {"ebreak", 0b1110011}
};
