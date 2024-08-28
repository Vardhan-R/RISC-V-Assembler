#include<iostream>
#include<bits/stdc++.h>
#include<sstream>
#include<fstream>
#include "RISCV_instr_constants.h"

using namespace std;

vector<string> final_hexcode;

// trim leading and trailing whitespaces and special characters
string trim(string& str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == string::npos)
        return "";

    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, (last - first + 1));
}

// process labels and store them in a map
void processLabels(vector<string>& input, unordered_map<string, int>& labels, vector<string>& instructions) {
    int program_cntr = 0;
    instructions.clear();

    for (size_t i = 0; i < input.size(); ++i) {
        string line = input[i];
        if (!line.empty()) { 

            size_t colon_pos = line.find(':');
            if (colon_pos != string::npos) { // if line contains a label
                string label = line.substr(0, colon_pos);
                line = line.substr(colon_pos + 1); // line after label
                labels[label] = program_cntr;
            }

            if (!line.empty()){
                instructions.push_back(line);
                program_cntr += 4;
            }

        }
    }
}

// handle brackets for load and store instructions
void processBrackets(vector<string> &tokens){
    for(auto &token: tokens){
        size_t pos = token.find('(');
        // for instructions like 100(x1)
        // extracts x1, 100 and removes ( and )
        if(pos != string::npos){
            int k = token.length();
            string imm = token.substr(0, pos);
            string reg = token.substr(pos+1, k-pos-2);
            tokens.pop_back();
            tokens.push_back(reg);
            tokens.push_back(imm);
        }
    }
}

// convert to lowercase
string toLower(string &str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// extract bits from num from num[high:low] both inclusive and transpose to left by transpose
int extractBits(int num, int high, int low, int transpose){
    return (num >> low & ((1 << (high - low + 1)) - 1)) << transpose;
}

// extract number from dec, bin, hex represenataion
int eval(string &num){
    int imm = 0;
    if(num[1] =='x'){
        imm = stoi(num, nullptr, 16);
    }
    else if (num[1] == 'b'){
        imm = stoi(num, nullptr, 2);
    }
    else{
        imm = stoi(num);
    }
    return imm;
}

void machineCode(vector<string> &tokens, unordered_map<string, int> &labels, int program_cntr){
    stringstream ss;
    string opcode = toLower(tokens[0]);

    int type = inst_type[opcode];
    vector<string> args(tokens.begin() + 1, tokens.end());
    int machine_code = 0;
    int rd, rs1, rs2;
    int imm;
    string hex_code;
    string label;

    switch(type){
        case R:
            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
            } 
            else {
                rd = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs1 = stoi(args[1].substr(1));
            } 
            else {
                rs1 = alias_to_ind[args[1]];
            }

            if (args[2][0] == 'x') {
                rs2 = stoi(args[2].substr(1));
            } 
            else {
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
            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
            }
            else {
                rd = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs1 = stoi(args[1].substr(1));
            }
            else {
                rs1 = alias_to_ind[args[1]];
            }
            imm = eval(args[2]);
            imm = extractBits(imm, 11, 0, 0);

            if(funct6_table[opcode] != -1){
                imm = extractBits(imm, 5, 0, 0);
                imm += funct6_table[opcode] * (1 << 6);
            }
            machine_code = (imm * (1<<20) + rs1 * (1<<15) +
                            funct3_table[opcode] * (1<<12) +
                            rd * (1<<7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case S:
            if (args[0][0] == 'x') {
                rs2 = stoi(args[0].substr(1));
            }
            else {
                rs2 = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs1 = stoi(args[1].substr(1));
            }
            else {
                rs1 = alias_to_ind[args[1]];
            }
            imm = eval(args[2]);
            machine_code = (extractBits(imm, 11, 5, 0) * (1<<25) +
                            rs2 * (1<<20) + rs1 * (1<<15) +
                            funct3_table[opcode] * (1<<12) +
                            extractBits(imm, 4, 0, 0) * (1<<7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case B:
            if (args[0][0] == 'x') {
                rs1 = stoi(args[0].substr(1));
            }
            else {
                rs1 = alias_to_ind[args[0]];
            }

            if (args[1][0] == 'x') {
                rs2 = stoi(args[1].substr(1));
            }
            else {
                rs2 = alias_to_ind[args[1]];
            }
            label = args[2];
            imm = labels[label] - program_cntr;

            machine_code = (extractBits(imm, 12, 12, 0) * (1<<31) +
                            extractBits(imm, 10, 5, 0) * (1<<25) +
                            rs2 * (1<<20) + rs1 * (1<<15) +
                            funct3_table[opcode] * (1<<12) +
                            extractBits(imm, 4, 1, 0) * (1<<8) +
                            extractBits(imm, 11, 11, 0) * (1<<7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case U:
            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
            }
            else {
                rd = alias_to_ind[args[0]];
            }

            imm = eval(args[1]);
            imm = imm << 12;
            machine_code = (extractBits(imm, 31, 12, 0) * (1<<12) +
                            rd * (1<<7) + opcode_table[opcode]);

            ss << hex << setw(8) << setfill('0') << machine_code;
            hex_code = ss.str();
            final_hexcode.push_back(hex_code);
            break;

        case J:
            if (args[0][0] == 'x') {
                rd = stoi(args[0].substr(1));
            }
            else {
                rd = alias_to_ind[args[0]];
            }
            label = args[1];
            imm = labels[label] - program_cntr;
            machine_code = (extractBits(imm, 20, 20, 0) * (1<<31) +
                            extractBits(imm, 10, 1, 0) * (1<<21) +
                            extractBits(imm, 11, 11, 0) * (1<<20) +
                            extractBits(imm, 19, 12, 0) * (1<<12) +
                            rd * (1<<7) + opcode_table[opcode]);

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
    while (getline(input_file, line)) {
        line = trim(line);
        size_t dot_pos = line.find('.');
        if (dot_pos != string::npos){
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

        if(!line.empty()){
            input.push_back(line);
        }
    }
    input_file.close();

    // for(auto &line: input){
    //     cout << line << endl;
    // }
    // cout<<endl;

    unordered_map<string, int> labels; // store labels
    vector<string> instructions; // store extracted instructions
    processLabels(input, labels, instructions); // find locations of label

    // for(auto line : instructions) {
    //     cout << line << endl;
    // }
    // cout << endl;

    vector<vector<string>> output; // for testing output
    int program_cntr = 0;

    for (auto line : instructions) {
        if (!line.empty()) {
            replace(line.begin(), line.end(), ',', ' ');

            istringstream iss(line); // remove white spaces
            string token;
            vector<string> tokens;
            // tokens is vector of strings that has to be processed
            while (iss >> token) {
                tokens.push_back(token);
            }

            processBrackets(tokens);
            // for(auto token : tokens) {
            //     cout << token << "-";
            // }
            // cout << endl;
            output.push_back(tokens); // testing parsed tokens
            machineCode(tokens, labels, program_cntr);
            program_cntr += 4;
        }
    }

    for(auto &code: final_hexcode) {
        cout << code << endl;
    }



    // for (auto vec : output) {
    //     for(auto token : vec) {
    //         cout << token << "-";
    //     }
    //     cout << endl;
    // }



    // cout << endl;
    // for (auto l : labels) {
    //     cout << l.first << " " << l.second << endl;
    // }

    ofstream output_file("output.hex");
    for (auto& code : final_hexcode) {
        output_file << code << endl;
    }
    output_file.close();
    return 0;
}

