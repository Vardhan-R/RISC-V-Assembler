import sys

# class Execute:
# 	def doNothing(*args) -> None:
# 		pass

# 	def add(*args) -> None:
# 		# global regs
# 		# rd, rs1, rs2 = args
# 		# regs[rd] = regs[rs1] + regs[rs2]
# 		# if
# 		pass

# 	def sub(*args) -> None:
# 		pass

class InstType:
	R, I, S, B, U, J = list(range(6))

def convertToMachineCode(tokens: list[str], program_cntr: int, num_sys: str = "bin") -> str:
	opcode = tokens[0].lower()
	args = tokens[1:]

	match inst_types[opcode]:
		case InstType.R:
			if args[0][0] == 'x':
				rd = int(args[0][1:])
			else:
				rd = alias_to_ind[args[0]]

			if args[1][0] == 'x':
				rs1 = int(args[1][1:])
			else:
				rs1 = alias_to_ind[args[1]]

			if args[2][0] == 'x':
				rs2 = int(args[2][1:])
			else:
				rs2 = alias_to_ind[args[2]]

			machine_code = (funct7_table[opcode] * 2 ** 25
							+ rs2 * 2 ** 20 + rs1 * 2 ** 15
							+ funct3_table[opcode] * 2 ** 12
							+ rd * 2 ** 7 + opcode_table[opcode])

		case InstType.I:
			# quick fix for `ecall`
			if opcode == "ecall":
				machine_code = 0x73
			else:
				if opcode[0] == 'l':
					args.insert(1, args.pop())

				if args[0][0] == 'x':
					rd = int(args[0][1:])
				else:
					rd = alias_to_ind[args[0]]

				if args[1][0] == 'x':
					rs1 = int(args[1][1:])
				else:
					rs1 = alias_to_ind[args[1]]

				imm = ''.join(args[2:])
				imm = eval(imm, labels) # have to make it right to left (ignoring order of ops)
				imm = extractBits(num=imm, high=11, low=0, transpose=0)
				if funct6_table[opcode] is not None:
					match opcode:
						case "ecall":
							imm = 0x0
						case "ebreak":
							imm = 0x1
						case _:
							imm = extractBits(num=imm, high=5, low=0, transpose=0) # report error if `imm` is not in range
							imm += funct6_table[opcode] * 2 ** 6

				machine_code = (imm * 2 ** 20 + rs1 * 2 ** 15
								+ funct3_table[opcode] * 2 ** 12
								+ rd * 2 ** 7 + opcode_table[opcode])

		case InstType.S:
			args.insert(1, args.pop())

			if args[0][0] == 'x':
				rs2 = int(args[0][1:])
			else:
				rs2 = alias_to_ind[args[0]]

			if args[1][0] == 'x':
				rs1 = int(args[1][1:])
			else:
				rs1 = alias_to_ind[args[1]]

			imm = eval(''.join(args[2:])) # check range

			machine_code = (extractBits(num=imm, high=11, low=5, transpose=25)
							+ rs2 * 2 ** 20
							+ rs1 * 2 ** 15
							+ funct3_table[opcode] * 2 ** 12
							+ extractBits(num=imm, high=4, low=0, transpose=7)
							+ opcode_table[opcode])

		case InstType.B:
			if args[0][0] == 'x':
				rs1 = int(args[0][1:])
			else:
				rs1 = alias_to_ind[args[0]]

			if args[1][0] == 'x':
				rs2 = int(args[1][1:])
			else:
				rs2 = alias_to_ind[args[1]]

			imm = ''.join(args[2:])
			try:
				imm = int(imm)
			except:
				imm = eval(imm, labels) # have to make it right to left (ignoring order of ops)
				imm -= program_cntr
			imm = extractBits(num=imm, high=12, low=1, transpose=1)

			machine_code = (extractBits(num=imm, high=12, low=12, transpose=31)
							+ extractBits(num=imm, high=10, low=5, transpose=25)
							+ rs2 * 2 ** 20
							+ rs1 * 2 ** 15
							+ funct3_table[opcode] * 2 ** 12
							+ extractBits(num=imm, high=4, low=1, transpose=8)
							+ extractBits(num=imm, high=11, low=11, transpose=7)
							+ opcode_table[opcode])

		case InstType.U:
			if args[0][0] == 'x':
				rd = int(args[0][1:])
			else:
				rd = alias_to_ind[args[0]]

			imm = eval(''.join(args[1:])) # have to make it right to left (ignoring order of ops)
			imm = extractBits(num=imm, high=19, low=0, transpose=12)

			machine_code = (imm
							+ rd * 2 ** 7
							+ opcode_table[opcode])

		case InstType.J:
			if args[0][0] == 'x':
				rd = int(args[0][1:])
			else:
				rd = alias_to_ind[args[0]]

			imm = ''.join(args[1:])
			try:
				imm = int(imm)
			except:
				imm = eval(imm, labels) # have to make it right to left (ignoring order of ops)
				imm -= program_cntr
			imm = (extractBits(num=imm, high=20, low=20, transpose=31)
				+ extractBits(num=imm, high=10, low=1, transpose=21)
				+ extractBits(num=imm, high=11, low=11, transpose=20)
				+ extractBits(num=imm, high=19, low=12, transpose=12))

			machine_code = (imm
							+ rd * 2 ** 7
							+ opcode_table[opcode])

		case _:
			num_sys = None

	match num_sys:
		case "bin":
			machine_code = bin(machine_code)[2:]
			return '0' * (32 - len(machine_code)) + machine_code
		case "dec":
			return str(machine_code)
		case "hex":
			machine_code = hex(machine_code)[2:]
			return '0' * (8 - len(machine_code)) + machine_code
		case _:
			return ""

def extractBits(num: int, low: int, high: int, transpose: int = 0) -> int:
	"""
	Extract low to high (both inclusive).
	"""

	return (num >> low & (2 ** (high - low + 1) - 1)) << transpose

def firstPass(lines: list[str]) -> tuple[dict[str, int], list[str]]:
	labels = {}
	program_cntr = 0

	for i, line in enumerate(lines):
		if line != '': # ignore blank lines
			if ':' in line: # label
				parts = line.split(':')
				labels[parts[0].strip()] = program_cntr
				lines[i] = parts[-1].split('#')[0].strip()
				if lines[i] != '':
					program_cntr += 4
			elif '.' not in line:
				program_cntr += 4

	return labels, lines

def removeOutermostParentheses(s: str) -> str:
	"""
	Assumes that the parentheses are balanced.
	"""

	cnt, ind, l = 0, 0, len(s)

	while ind < l:
		if s[ind] == '(':
			if cnt == 0:
				s = f"{s[:ind]} {s[ind + 1:]}"
			cnt += 1
		elif s[ind] == ')':
			cnt -= 1
			if cnt == 0:
				s = f"{s[:ind]} {s[ind + 1:]}"
		ind += 1

	return s

def secondPass(lines: list[str]) -> list[str]:
	global labels

	machine_codes = []
	program_cntr = 0

	for line in lines:
		if line != '': # ignore blank lines
			line = line.replace(',', ' ')
			line = removeOutermostParentheses(line)
			tokens = [token for token in line.split(' ') if token != '']
			try:
				machine_code = convertToMachineCode(tokens, program_cntr, num_sys="hex")
			except:
				print("Error in line:", line)
				machine_code = ' '
			if machine_code != '':
				machine_codes.append(machine_code)
				program_cntr += 4

	return machine_codes

# mem_text = [0]
# mem_data = [0]
# mem_bss = [0]
# len_text = 8
# len_data = 8
# len_bss = 8
# regs = [0] * 32
# regs[2] = 0x000000007ffffff0
# regs[3] = 0x0000000010000000

alias_to_ind = {"zero": 0,
				"ra": 1,
				"sp": 2,
				"gp": 3,
				"tp": 4,
				"t0": 5,
				"t1": 6,
				"t2": 7,
				"s0": 8,
				"fp": 8,
				"s1": 9,
				"a0": 10,
				"a1": 11,
				"a2": 12,
				"a3": 13,
				"a4": 14,
				"a5": 15,
				"a6": 16,
				"a7": 17,
				"s2": 18,
				"s3": 19,
				"s4": 20,
				"s5": 21,
				"s6": 22,
				"s7": 23,
				"s8": 24,
				"s9": 25,
				"s10": 26,
				"s11": 27,
				"t3": 28,
				"t4": 29,
				"t5": 30,
				"t6": 31}

inst_types = {".data": -1,
				".text": -1,
				".bss": -1,
				".byte": -1,
				".half": -1,
				".word": -1,
				".dword": -1,

				"add": InstType.R,
				"sub": InstType.R,
				"xor": InstType.R,
				"or": InstType.R,
				"and": InstType.R,
				"sll": InstType.R,
				"srl": InstType.R,
				"sra": InstType.R,
				"slt": InstType.R,
				"sltu": InstType.R,

				"addi": InstType.I,
				"xori": InstType.I,
				"ori": InstType.I,
				"andi": InstType.I,
				"slli": InstType.I,
				"srli": InstType.I,
				"srai": InstType.I,
				"slti": InstType.I,
				"sltiu": InstType.I,

				"lb": InstType.I,
				"lh": InstType.I,
				"lw": InstType.I,
				"ld": InstType.I,
				"lbu": InstType.I,
				"lhu": InstType.I,
				"lwu": InstType.I,

				"sb": InstType.S,
				"sh": InstType.S,
				"sw": InstType.S,
				"sd": InstType.S,

				"beq": InstType.B,
				"bne": InstType.B,
				"blt": InstType.B,
				"bge": InstType.B,
				"bltu": InstType.B,
				"bgeu": InstType.B,

				"jal": InstType.J,
				"jalr": InstType.I,

				"lui": InstType.U,
				"auipc": InstType.U,

				"ecall": InstType.I,

				"ebreak": InstType.I}

opcode_table = {"add": 0b0110011,
				"sub": 0b0110011,
				"xor": 0b0110011,
				"or": 0b0110011,
				"and": 0b0110011,
				"sll": 0b0110011,
				"srl": 0b0110011,
				"sra": 0b0110011,
				"slt": 0b0110011,
				"sltu": 0b0110011,

				"addi": 0b0010011,
				"xori": 0b0010011,
				"ori": 0b0010011,
				"andi": 0b0010011,
				"slli": 0b0010011,
				"srli": 0b0010011,
				"srai": 0b0010011,
				"slti": 0b0010011,
				"sltiu": 0b0010011,

				"lb": 0b0000011,
				"lh": 0b0000011,
				"lw": 0b0000011,
				"ld": 0b0000011,
				"lbu": 0b0000011,
				"lhu": 0b0000011,
				"lwu": 0b0000011,

				"sb": 0b0100011,
				"sh": 0b0100011,
				"sw": 0b0100011,
				"sd": 0b0100011,

				"beq": 0b1100011,
				"bne": 0b1100011,
				"blt": 0b1100011,
				"bge": 0b1100011,
				"bltu": 0b1100011,
				"bgeu": 0b1100011,

				"jal": 0b1101111,
				"jalr": 0b1100111,

				"lui": 0b0110111,
				"auipc": 0b0010111,

				"ecall": 0b1110011,

				"ebreak": 0b1110011}

funct3_table = {"add": 0x0,
				"sub": 0x0,
				"xor": 0x4,
				"or": 0x6,
				"and": 0x7,
				"sll": 0x1,
				"srl": 0x5,
				"sra": 0x5,
				"slt": 0x2,
				"sltu": 0x3,

				"addi": 0x0,
				"xori": 0x4,
				"ori": 0x6,
				"andi": 0x7,
				"slli": 0x1,
				"srli": 0x5,
				"srai": 0x5,
				"slti": 0x2,
				"sltiu": 0x3,

				"lb": 0x0,
				"lh": 0x1,
				"lw": 0x2,
				"ld": 0x3,
				"lbu": 0x4,
				"lhu": 0x5,
				"lwu": 0x6,

				"sb": 0x0,
				"sh": 0x1,
				"sw": 0x2,
				"sd": 0x3,

				"beq": 0x0,
				"bne": 0x1,
				"blt": 0x4,
				"bge": 0x5,
				"bltu": 0x6,
				"bgeu": 0x7,

				"jalr": 0x0,

				"ecall": 0x0,

				"ebreak": 0x0}

funct6_table = {"addi": None,
				"xori": None,
				"ori": None,
				"andi": None,
				"slli": 0x00,
				"srli": 0x00,
				"srai": 0x10,
				"slti": None,
				"sltiu": None,

				"lb": None,
				"lh": None,
				"lw": None,
				"ld": None,
				"lbu": None,
				"lhu": None,
				"lwu": None,

				"jalr": None,

				"ecall": 0x0,

				"ebreak": 0x1}

funct7_table = {"add": 0x00,
				"sub": 0x20,
				"xor": 0x00,
				"or": 0x00,
				"and": 0x00,
				"sll": 0x00,
				"srl": 0x00,
				"sra": 0x20,
				"slt": 0x00,
				"sltu": 0x00}

# opcode_table = {".data": -1,
# 				".text": -1,
# 				".bss": -1,
# 				".byte": -1,
# 				".half": -1,
# 				".word": -1,
# 				".dword": -1,
# 				"add": Execute.add,
# 				"sub": -1,
# 				"xor": -1,
# 				"or": -1,
# 				"and": -1,
# 				"sll": -1,
# 				"srl": -1,
# 				"sra": -1,
# 				"slt": -1,
# 				"sltu": -1}

source_file = sys.argv[1]

with open(source_file, 'r') as fp:
	raw_lines = fp.readlines()

raw_lines = list(map(lambda s: s.replace(';', '#'), raw_lines))
formatted_lines = list(map(lambda s: s.split('#')[0].strip(), raw_lines))

labels, new_lines = firstPass(formatted_lines)
label_names = list(labels.keys())

print({name: hex(labels[name]) for name in label_names}, '\n')

machine_codes = secondPass(new_lines)

# print('\n'.join(machine_codes))

with open("./py_output.txt", 'w') as fp:
	fp.write('\n'.join(machine_codes))

# compare
# with open("./ground_truth.txt", 'r') as fp:
with open("./output.hex", 'r') as fp:
	raw_lines = fp.readlines()

gt = [line.strip().split(' ')[-1] for line in raw_lines if line.strip() != '' and '<' not in line]
for i, (tr, my) in enumerate(zip(gt, machine_codes)):
	if tr == my:
		print(hex(4 * i)[2:])
		pass
	else:
		print(f"{hex(4 * i)[2:]}	{tr}	{my}")

print("End of program.")

"""
Known errors:
	doesn't work too well with syntactically incorrect input
	can't report errors
	v can't work with bracket syntax
	can't work with divisions in `imm`
	v has to work with various number systems in mathematical expressions
	v auipc x3 65536 works but auipc x3 0x10000 does not
	v branch and jump can't work with expressions in terms of labels (such as `label_1` + `label_2`)
"""
