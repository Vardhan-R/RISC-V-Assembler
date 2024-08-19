#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <bitset>

using namespace std;

enum class InstType { W, R, I, S, B, U, J }; // W ==> waste

map<string, int> alias_to_ind = {
    {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4},
    {"t0", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"fp", 8},
    {"s1", 9}, {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13},
    {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18},
    {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28},
    {"t4", 29}, {"t5", 30}, {"t6", 31}
};

map<string, InstType> inst_types = {
    {".data", InstType::W}, {".text", InstType::W}, {".bss", InstType::W},
    {"add", InstType::R}, {"sub", InstType::R}, {"xor", InstType::R},
    {"or", InstType::R}, {"and", InstType::R}, {"sll", InstType::R},
    {"srl", InstType::R}, {"sra", InstType::R}, {"slt", InstType::R},
    {"sltu", InstType::R}, {"addi", InstType::I}, {"xori", InstType::I},
    {"ori", InstType::I}, {"andi", InstType::I}, {"slli", InstType::I},
    {"srli", InstType::I}, {"srai", InstType::I}, {"slti", InstType::I},
    {"sltiu", InstType::I}, {"lb", InstType::I}, {"lh", InstType::I},
    {"lw", InstType::I}, {"ld", InstType::I}, {"lbu", InstType::I},
    {"lhu", InstType::I}, {"lwu", InstType::I}, {"sb", InstType::S},
    {"sh", InstType::S}, {"sw", InstType::S}, {"sd", InstType::S},
    {"beq", InstType::B}, {"bne", InstType::B}, {"blt", InstType::B},
    {"bge", InstType::B}, {"bltu", InstType::B}, {"bgeu", InstType::B},
    {"jal", InstType::J}, {"jalr", InstType::I}, {"lui", InstType::U},
    {"auipc", InstType::U}, {"ecall", InstType::I}, {"ebreak", InstType::I}
};

map<string, int> opcode_table = {
    {"add", 0b0110011}, {"sub", 0b0110011}, {"xor", 0b0110011},
    {"or", 0b0110011}, {"and", 0b0110011}, {"sll", 0b0110011},
    {"srl", 0b0110011}, {"sra", 0b0110011}, {"slt", 0b0110011},
    {"sltu", 0b0110011}, {"addi", 0b0010011}, {"xori", 0b0010011},
    {"ori", 0b0010011}, {"andi", 0b0010011}, {"slli", 0b0010011},
    {"srli", 0b0010011}, {"srai", 0b0010011}, {"slti", 0b0010011},
    {"sltiu", 0b0010011}, {"lb", 0b0000011}, {"lh", 0b0000011},
    {"lw", 0b0000011}, {"ld", 0b0000011}, {"lbu", 0b0000011},
    {"lhu", 0b0000011}, {"lwu", 0b0000011}, {"sb", 0b0100011},
    {"sh", 0b0100011}, {"sw", 0b0100011}, {"sd", 0b0100011},
    {"beq", 0b1100011}, {"bne", 0b1100011}, {"blt", 0b1100011},
    {"bge", 0b1100011}, {"bltu", 0b1100011}, {"bgeu", 0b1100011},
    {"jal", 0b1101111}, {"jalr", 0b1100111}, {"lui", 0b0110111},
    {"auipc", 0b0010111}, {"ecall", 0b1110011}, {"ebreak", 0b1110011}
};

map<string, int> funct3_table = {
    {"add", 0x0}, {"sub", 0x0}, {"xor", 0x4}, {"or", 0x6},
    {"and", 0x7}, {"sll", 0x1}, {"srl", 0x5}, {"sra", 0x5},
    {"slt", 0x2}, {"sltu", 0x3}, {"addi", 0x0}, {"xori", 0x4},
    {"ori", 0x6}, {"andi", 0x7}, {"slli", 0x1}, {"srli", 0x5},
    {"srai", 0x5}, {"slti", 0x2}, {"sltiu", 0x3}, {"lb", 0x0},
    {"lh", 0x1}, {"lw", 0x2}, {"ld", 0x3}, {"lbu", 0x4},
    {"lhu", 0x5}, {"lwu", 0x6}, {"sb", 0x0}, {"sh", 0x1},
    {"sw", 0x2}, {"sd", 0x3}, {"beq", 0x0}, {"bne", 0x1},
    {"blt", 0x4}, {"bge", 0x5}, {"bltu", 0x6}, {"bgeu", 0x7},
    {"jalr", 0x0}, {"ecall", 0x0}, {"ebreak", 0x0}
};

map<string, int> funct6_table = {
    {"addi", -1}, {"xori", -1}, {"ori", -1}, {"andi", -1},
    {"slli", 0x00}, {"srli", 0x00}, {"srai", 0x10}, {"slti", -1},
    {"sltiu", -1}, {"lb", -1}, {"lh", -1}, {"lw", -1},
    {"ld", -1}, {"lbu", -1}, {"lhu", -1}, {"lwu", -1},
    {"jalr", -1}, {"ecall", 0x0}, {"ebreak", 0x1}
};

map<string, int> funct7_table = {
    {"add", 0x00}, {"sub", 0x20}, {"xor", 0x00}, {"or", 0x00},
    {"and", 0x00}, {"sll", 0x00}, {"srl", 0x00}, {"sra", 0x20},
    {"slt", 0x00}, {"sltu", 0x00}
};

int extractBits(int num, int low, int high, int transpose = 0) {
    return ((num >> low) & ((1 << (high - low + 1)) - 1)) << transpose;
}

void removeOutermostParentheses(string &s) {
    int cnt = 0;
    for (auto it = s.begin(); it != s.end();) {
        if (*it == '(') {
            if (cnt == 0) it = s.erase(it);
            else ++it;
            ++cnt;
        } else if (*it == ')') {
            --cnt;
            if (cnt == 0) it = s.erase(it);
            else ++it;
        } else {
            ++it;
        }
    }
}

map<string, int> firstPass(vector<string> &lines) {
    map<string, int> labels;
    int program_cntr = 0;

    for (string &line : lines) {
        int colon = line.find(':');
        if (colon != string::npos) {
            labels[line.substr(0, colon)] = program_cntr;
        } else {
            ++program_cntr;
        }
    }
    return labels;
}

pair<pair<string, string>, InstType> instructionTypeAndParams(string &line) {
    string instruction;
    string rest;

    istringstream iss(line);
    iss >> instruction;
    getline(iss, rest);

    int ty = static_cast<int>(inst_types[instruction]);

    // cout << "Instruction mnemonic: " << instruction << endl;
    // cout << "Instruction type " << ty << endl;
    // cout << "Rest of the line: " << rest << endl;

    for (int i = 0; i < rest.length(); ++i) {
        if (rest[i] == ',' || rest[i] == '(' || rest[i] == ')') {
            rest[i] = ' ';
        }
    }

    return {{instruction, rest}, inst_types[instruction]};
}

// vector<int> paramsForInstruction(InstType inst_type, string &line, map<string, int> &labels) {
//     vector<int> params;
//     string instruction, param;
//     istringstream iss(line);

//     iss >> instruction;
//     while (iss >> param) {
//         params.push_back((param[0] == 'x') ? stoi(param.substr(1)) : ((isdigit(param[0]) || param[0] == '-') ? stoi(param) : alias_to_ind[param]));
//     }

//     if (inst_type == InstType::B || inst_type == InstType::J) {
//         params.back() = labels[param] - labels[line];
//     }

//     return params;
// }

// vector<int> paramsForInstruction(InstType inst_type, string &line, map<string, int> &labels) {
//     vector<int> params;
//     string instruction, param;
//     istringstream iss(line);

//     // Extract the instruction mnemonic
//     iss >> instruction;

//     // Extract parameters
//     while (iss >> param) {
//         if (param.empty()) {
//             continue; // Skip empty parameters
//         }
//         // Handle registers, numbers, and aliases
//         if (param[0] == 'x') {
//             params.push_back(stoi(param.substr(1))); // Convert register number
//         } else if (isdigit(param[0]) || param[0] == '-') {
//             params.push_back(stoi(param)); // Convert numeric literals
//         } else {
//             params.push_back(alias_to_ind[param]); // Convert aliases to indices
//         }
//     }

//     // Special handling for branch and jump instructions
//     if (inst_type == InstType::B || inst_type == InstType::J) {
//         // Update the last parameter to be the difference between labels
//         // Assuming last parameter is a label
//         if (!params.empty()) {
//             string last_param = param; // Store last param (label)
//             params.back() = labels[last_param] - labels[line]; // Calculate difference
//         }
//     }

//     return params;
// }

vector<int> paramsForInstruction(InstType inst_type, string &line, map<string, int> &labels) {
    vector<int> params;
    string param;
    istringstream iss(line);

    // // Extract the instruction mnemonic
    // iss >> instruction;

    // Debug print for instruction
    // cout << "Instruction: " << instruction << endl;

    // Extract parameters
    while (iss >> param) {
        // Debug print for each parameter
        // cout << "Parameter: " << param << endl;

        if (param.empty()) {
            continue; // Skip empty parameters
        }
        // Handle registers, numbers, and aliases
        if (param[0] == 'x') {
            params.push_back(stoi(param.substr(1))); // Convert register number
        } else if (isdigit(param[0]) || param[0] == '-') {
            params.push_back(stoi(param)); // Convert numeric literals
        } else {
            params.push_back(alias_to_ind[param]); // Convert aliases to indices
        }
    }

    // Special handling for branch and jump instructions
    if (inst_type == InstType::B || inst_type == InstType::J) {
        if (!params.empty()) {
            // Assuming last parameter is a label
            string last_param = param; // Store last param (label)
            params.back() = labels[last_param] - labels[line]; // Calculate difference
        }
    }

    return params;
}

// int main() {
//     ifstream infile("source.s");
//     ofstream outfile("binary_output.txt");

//     vector<string> lines;
//     string line;
//     while (getline(infile, line)) {
//         lines.push_back(line);
//     }

//     map<string, int> labels = firstPass(lines);

//     for (string &line : lines) {
//         // Instead of structured bindings, define each variable separately
//         pair<InstType, string> instTypeAndParams = instructionTypeAndParams(line);
//         InstType inst_type = instTypeAndParams.first;
//         string rest = instTypeAndParams.second;

//         vector<int> params = paramsForInstruction(inst_type, rest, labels);

//         int instruction = opcode_table[rest];
//         instruction |= funct3_table[rest] << 12;
//         instruction |= alias_to_ind[rest] << 7;

//         if (inst_type == InstType::R) {
//             instruction |= funct7_table[rest] << 25;
//         } else if (inst_type == InstType::I) {
//             instruction |= (funct6_table[rest] << 25) | (params[2] << 20);
//         }

//         bitset<32> binary(instruction);
//         outfile << binary << endl;
//     }

//     infile.close();
//     outfile.close();

//     return 0;
// }

int main() {
    ifstream infile("source.s");
    ofstream outfile("binary_output.txt");

    vector<string> lines;
    string line;
    while (getline(infile, line)) {
        lines.push_back(line);
    }

    map<string, int> labels = firstPass(lines);

    for (string &line : lines) {
        // // Debug output: show the line being processed
        // cout << "Processing line: " << line << endl;

        pair<pair<string, string>, InstType> instTypeAndParams = instructionTypeAndParams(line);
        string inst = instTypeAndParams.first.first;
        string rest = instTypeAndParams.first.second;
        InstType inst_type = instTypeAndParams.second;

        vector<int> params = paramsForInstruction(inst_type, rest, labels);

        // // Debug output: show parsed instruction details
        // cout << "Instruction Type: " << static_cast<int>(inst_type) << endl;
        // cout << "Rest of the line: " << rest << endl;
        // cout << "Parameters: ";
        // for (int param : params) {
        //     cout << param << " ";
        // }
        // cout << endl;

        int instruction = opcode_table[inst];

        if (inst_type == InstType::R) {
            instruction |= params[0] << 7;
            instruction |= funct3_table[inst] << 12;
            instruction |= params[1] << 15;
            instruction |= params[2] << 20;
            instruction |= funct7_table[inst] << 25;
        } else if (inst_type == InstType::I) {
            instruction |= params[0] << 7;
            instruction |= funct3_table[inst] << 12;
            instruction |= params[1] << 15;
            if (inst == "slli" || inst == "srli") {
                params[2] %= 64;
            } else if (inst == "srai") {
                params[2] = 1024 + params[2] % 64;
            } else if (inst == "ecall") {
                params[2] = 0x0;
            } else if (inst == "ebreak") {
                params[2] = 0x1;
            }
            instruction |= params[2] << 20;
        } else if (inst_type == InstType::S) {
            instruction |= (params[1] & 0b11111) << 7;
            instruction |= funct3_table[inst] << 12;
            instruction |= params[2] << 15;
            instruction |= params[0] << 20;
            instruction |= ((params[1] >> 5) & 0b1111111) << 25;
        } else if (inst_type == InstType::B) {
        } else if (inst_type == InstType::U) {
            instruction |= params[0] << 7;
            instruction |= (params[1] & 0b11111111111111111111) << 12;
        } else if (inst_type == InstType::J) {
        }

        // Debug output: show the constructed binary instruction
        // cout << "Constructed Bin Instruction: " << bitset<32>(instruction) << endl << endl;
        // cout << "Constructed Hex Instruction: " << std::hex << instruction << endl << endl;
        if (instruction != 0) {
            cout << std::hex << instruction << endl;

            bitset<32> binary(instruction);
            outfile << binary << endl;
        }
    }

    infile.close();
    outfile.close();

    return 0;
}

/*
U is not working for hex input
*/
