#include<iostream>
#include<bits/stdc++.h>
#include<sstream>
#include<fstream>
#include "RISCV_instr_constants.h"

using namespace std;

vector<string> final_hexcode;

// Trim leading and trailing whitespaces and special characters
string trim(string &str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == string::npos)
        return "";

    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, (last - first + 1));
}

// Process labels and find the adress of labels
void processLabels(vector<string> &input, unordered_map<string, int> &labels, vector<string> &instructions, vector<int> &line_num, vector<int> &line_numprocessed) {
    int program_cntr = 0;
    instructions.clear();

    for (int i = 0; i < input.size(); ++i) {
        string line = input[i];
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {    // If line contains a label
            string label = line.substr(0, colon_pos);
            label = trim(label);    // see (should not start with a digit)
            line = line.substr(colon_pos + 1);    // line after label
            line = trim(line);
            labels[label] = program_cntr;
        }

        if (!line.empty()) {
            line_numprocessed.push_back(line_num[i]);
            instructions.push_back(line);
            program_cntr += 4;
        }
    }
}

// Handle brackets for load and store instructions
void processBrackets(vector<string> &tokens) {
    for (auto &token: tokens) {
        size_t pos = token.find('(');
        // for instructions like 100(x1)
        // extracts x1, 100 and removes ( and )
        if (pos != string::npos) {
            int k = token.length();
            string imm = token.substr(0, pos);
            string reg = token.substr(pos + 1, k - pos - 2);    // see
            tokens.pop_back();
            tokens.push_back(reg);
            tokens.push_back(imm);
        }
    }
}

// Convert to lowercase
string toLower(string &str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// Extract bits from num from num[high:low] both inclusive and transpose to left by transpose
int extractBits(int num, int high, int low, int transpose) {
    return (num >> low & ((1 << (high - low + 1)) - 1)) << transpose;
}

// Extract number from dec, bin, hex represenataion and check if they are valid representations
int eval(string &num, int line) {
    int imm = 0;
    size_t pos;
    try {
        if (num[1] =='x') {
            imm = stoi(num, &pos, 16);
        }
        else if (num[1] == 'b') {
            imm = stoi(num.substr(2), &pos, 2);
            pos += 2;
        }
        else{
            imm = stoi(num);
            pos = num.length();
        }

        if (pos != num.length()) {
            cerr << "Error : " << "Line " << line << "| Invalid immediate value" << endl;
            exit(1);
        }
    }

    catch (invalid_argument &e) {
        cerr << "Error : " << "Line " << line << "| Invalid immediate value" << endl;
        exit(1);
    }
    return imm;
}

// Check if number of arguments are correct for given instruction
void checkArgLen(string &opcode, int arg_len, int len, int line) {
    if (arg_len != len) {
        cerr << "Error : " << "Line " << line << "| Invalid number of arguments for :" << opcode << endl;
        exit(1);
    }
}

// Check if register number is valid
void checkRegister(int reg, int line) {
    if (reg < 0 || reg > 31) {
        cerr << "Error : " << "Line " << line << "| Invalid Register Number" << endl;
        exit(1);
    }
}

// Check if register alias is valid
void checkRegAlias(string &reg, int line) {
    if (alias_to_ind.find(reg) == alias_to_ind.end()) {
        cerr << "Error : " << "Line " << line << "| Invalid Register Alias " << reg << endl;
        exit(1);
    }
}

// check if label is defined
void checkLabel(string &label, unordered_map<string, int> &labels, int line) {
    if (labels.find(label) == labels.end()) {
        cerr << "Error : " << "Line " << line << "| Label not defined: " << label << endl;
        exit(1);
    }
}

void machineCode(vector<string> &tokens, unordered_map<string, int> &labels, int program_cntr, int line) {
    stringstream ss;
    string opcode = toLower(tokens[0]);
    if (opcode_table.find(opcode) == opcode_table.end()) {
        cerr << "Error : " << "Line " << line << "| Invalid Instruction :" << opcode << endl;
        exit(1);
    }
    int type = inst_type[opcode];
    vector<string> args(tokens.begin() + 1, tokens.end());
    int machine_code = 0;
    int rd, rs1, rs2;
    int imm;
    string hex_code;
    string label;
    int arg_len = args.size();

    switch(type) {
        case R:
            checkArgLen(opcode, arg_len, 3, line);

            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
                checkRegister(rd, line);
            }
            else {
                checkRegAlias(args[0], line);
                rd = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs1 = stoi(args[1].substr(1));
                checkRegister(rs1, line);
            }
            else {
                checkRegAlias(args[1], line);
                rs1 = alias_to_ind[args[1]];
            }

            if (args[2][0] == 'x') {
                rs2 = stoi(args[2].substr(1));
                checkRegister(rs2, line);
            }
            else {
                checkRegAlias(args[2], line);
                rs2 = alias_to_ind[args[2]];
            }

            machine_code = (funct7_table[opcode] << 25)
                           + (rs2 << 20)
                           + (rs1 << 15)
                           + (funct3_table[opcode] << 12)
                           + (rd << 7)
                           + opcode_table[opcode];

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case I:
            checkArgLen(opcode, arg_len, 3, line);
            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
                checkRegister(rd, line);
            }
            else {
                checkRegAlias(args[0], line);
                rd = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs1 = stoi(args[1].substr(1));
                checkRegister(rs1, line);
            }
            else {
                checkRegAlias(args[1], line);
                rs1 = alias_to_ind[args[1]];
            }

            imm = eval(args[2], line);    // see (check range)
            imm = extractBits(imm, 11, 0, 0);

            if (funct6_table[opcode] != -1) {
                imm = extractBits(imm, 5, 0, 0);    // see (check range)
                imm += funct6_table[opcode] * (1 << 6);
            }
            machine_code = (imm * (1 << 20) + rs1 * (1 << 15) +
                            funct3_table[opcode] * (1 << 12) +
                            rd * (1 << 7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case S:
            checkArgLen(opcode, arg_len, 3, line);
            if (args[0][0] == 'x') {
                rs2 = stoi(args[0].substr(1));
                checkRegister(rs2, line);
            }
            else {
                checkRegAlias(args[0], line);
                rs2 = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs1 = stoi(args[1].substr(1));
                checkRegister(rs1, line);
            }
            else {
                checkRegAlias(args[1], line);
                rs1 = alias_to_ind[args[1]];
            }

            imm = eval(args[2], line);    // see (check range)

            machine_code = (extractBits(imm, 11, 5, 0) * (1 << 25) +
                            rs2 * (1 << 20) + rs1 * (1 << 15) +
                            funct3_table[opcode] * (1 << 12) +
                            extractBits(imm, 4, 0, 0) * (1 << 7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case B:
            checkArgLen(opcode, arg_len, 3, line);
            if (args[0][0] == 'x') {
                rs1 = stoi(args[0].substr(1));
                checkRegister(rs1, line);
            }
            else {
                checkRegAlias(args[0], line);
                rs1 = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs2 = stoi(args[1].substr(1));
                checkRegister(rs2, line);
            }
            else {
                checkRegAlias(args[1], line);
                rs2 = alias_to_ind[args[1]];
            }

            label = args[2];    // see (rel. jmp)
            checkLabel(label, labels, line);
            imm = labels[label] - program_cntr;

            machine_code = (extractBits(imm, 12, 12, 0) * (1 << 31) +
                            extractBits(imm, 10, 5, 0) * (1 << 25) +
                            rs2 * (1 << 20) + rs1 * (1 << 15) +
                            funct3_table[opcode] * (1 << 12) +
                            extractBits(imm, 4, 1, 0) * (1 << 8) +
                            extractBits(imm, 11, 11, 0) * (1 << 7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case U:
            checkArgLen(opcode, arg_len, 2, line);
            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
                checkRegister(rd, line);
            }
            else {
                checkRegAlias(args[0], line);
                rd = alias_to_ind[args[0]];
            }

            imm = eval(args[1], line);
            imm = imm << 12;
            machine_code = (extractBits(imm, 31, 12, 0) * (1 << 12) +
                            rd * (1 << 7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case J:
            checkArgLen(opcode, arg_len, 2, line);
            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
                checkRegister(rd, line);
            }
            else {
                checkRegAlias(args[0], line);
                rd = alias_to_ind[args[0]];
            }
            label = args[1];    // see (rel. jmp)
            checkLabel(label, labels, line);
            imm = labels[label] - program_cntr;

            machine_code = (extractBits(imm, 20, 20, 0) * (1 << 31) +
                            extractBits(imm, 10, 1, 0) * (1 << 21) +
                            extractBits(imm, 11, 11, 0) * (1 << 20) +
                            extractBits(imm, 19, 12, 0) * (1 << 12) +
                            rd * (1 << 7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Please provide assembly source file." << endl;
        return 1;
    }

    ifstream input_file(argv[1]);

    if (!input_file) {
        cerr << "Error: Could not open the file." << endl;
        return 1;
    }

    // obtain input from file
    vector<string> input;
    string line;
    vector<int> line_num;
    int l = 1;
    while (getline(input_file, line)) {
        line = trim(line);
        size_t dot_pos = line.find('.');
        if (dot_pos != string::npos) {
                l += 1;
                continue;
        }

        size_t comment_pos = line.find('#');
        if (comment_pos != string::npos) {
            line = line.substr(0, comment_pos);
            line = trim(line);
        }

        comment_pos = line.find(';');
        if (comment_pos != string::npos) {
            line = line.substr(0, comment_pos);
            line = trim(line);
        }

        if (!line.empty()) {
            input.push_back(line);
            line_num.push_back(l);
        }
        l += 1;
    }
    input_file.close();


    // for (auto j: line_num) {
    //     cout << j << " ";
    // }
    // cout << endl;


    // for (auto &line: input) {
    //     cout << line << endl;
    // }
    // cout << endl;

    unordered_map<string, int> labels;    // store labels
    vector<string> instructions;    // store extracted instructions
    vector<int> line_numprocessed;
    processLabels(input, labels, instructions, line_num, line_numprocessed);    // find locations of label

    // for (auto j: line_numprocessed) {
    //     cout << j << " ";
    // }
    // cout << endl;
    // cout << instructions.size() << endl;

    // for (auto line : instructions) {
    //     cout << line << endl;
    // }
    // cout << endl;

    vector<vector<string>> parsed_output;    // for testing output
    int program_cntr = 0;

    l = 0;
    for (auto line : instructions) {
        if (!line.empty()) {
            replace(line.begin(), line.end(), ',', ' ');

            istringstream iss(line);    // remove white spaces
            string token;
            vector<string> tokens;
            // tokens is vector of strings that has to be processed
            while (iss >> token) {
                tokens.push_back(token);
            }

            processBrackets(tokens);    // handle brackets for load and store instructions
            parsed_output.push_back(tokens);    // testing parsed tokens
            machineCode(tokens, labels, program_cntr, line_numprocessed[l]);
            program_cntr += 4;
            l += 1;
        }
    }

    // debugging
    // -d display hex code on terminal
    // -l display labels on terminal
    // -o display parsed tokens on terminal
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            for (auto &code: final_hexcode) {
                cout << code << endl;
            }
            cout << endl;
        }
        else if (strcmp(argv[i], "-l") == 0) {
            for (auto l : labels) {
                cout << l.first  << " " << l.second << endl;
            }
            cout << endl;
        }
        else if (strcmp(argv[i], "-o") == 0) {
            for (auto vec : parsed_output) {
                for (auto token : vec) {
                    cout << token << "-";
                }
                cout << endl;
            }
            cout << endl;
        }
    }



    ofstream output_file("output.hex");
    for (auto &code : final_hexcode) {
        output_file << code << endl;
    }
    output_file.close();
    cout << endl << "Output written to output.hex" << endl;
    return 0;
}
