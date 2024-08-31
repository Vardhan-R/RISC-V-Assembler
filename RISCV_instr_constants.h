#ifndef RISCV_INSTR_CONSTANTS_H
#define RISCV_INSTR_CONSTANTS_H

#include<iostream>
#include<bits/stdc++.h>
using namespace std;

enum InstrType {R, I, S, B, U, J};
extern unordered_map<string, int> inst_type;
extern unordered_map<string, int> alias_to_ind;
extern unordered_map<string, int> funct7_table;
extern unordered_map<string, int> funct3_table;
extern unordered_map<string, int> funct6_table;
extern unordered_map<string, int> opcode_table;

#endif
