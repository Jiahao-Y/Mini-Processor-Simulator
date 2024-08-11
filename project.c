/* CDA 3103 Project
 by Jiahao Yuan & Prisha Patel */

#include "spimcore.h" // Include the header file for definitions and structures

/* ALU */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {
    // Perform ALU operations based on the ALUControl signal
    switch (ALUControl) {
        case 0: // Addition
            *ALUresult = A + B; // Add operands A and B
            break;
        case 1: // Subtraction
            *ALUresult = A - B; // Subtract operand B from A
            break;
        case 2: // Set Less Than (Signed)
            *ALUresult = ((int)A < (int)B) ? 1 : 0; // Set result to 1 if A < B (signed), otherwise 0
            break;
        case 3: // Set Less Than (Unsigned)
            *ALUresult = (A < B) ? 1 : 0; // Set result to 1 if A < B (unsigned), otherwise 0
            break;
        case 4: // AND
            *ALUresult = (A & B) ? 1 : 0; // Perform bitwise AND on A and B, set result to 1 if non-zero
            break;
        case 5: // OR
            *ALUresult = (A | B) ? 1 : 0; // Perform bitwise OR on A and B, set result to 1 if non-zero
            break;
        case 6: // Shift Left by 16 Bits
            *ALUresult = B << 16; // Shift operand B left by 16 bits
            break;
        case 7: // NOT
            *ALUresult = ~A; // Perform bitwise NOT on A
            break;
        default:
            *ALUresult = 0; // Default case: set result to 0
            break;
    }
    // Set Zero flag to 1 if ALUresult is 0, otherwise 0
    *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* Instruction Fetch */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
    // Check if PC is word-aligned (PC must be a multiple of 4)
    if (PC % 4 != 0) {
        return 1; // Halt condition: PC is not word-aligned
    }
    // Fetch the instruction from memory
    *instruction = Mem[PC >> 2]; // Convert PC to index by dividing by 4
    return 0; // No halt condition
}

/* Instruction Partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec) {
    // Extract parts of the instruction using bitwise operations
    *op = (instruction >> 26) & 0x3F; // Extract opcode (6 bits from the left)
    *r1 = (instruction >> 21) & 0x1F; // Extract source register (5 bits)
    *r2 = (instruction >> 16) & 0x1F; // Extract target register (5 bits)
    *r3 = (instruction >> 11) & 0x1F; // Extract destination register (5 bits)
    *funct = instruction & 0x3F; // Extract function code (6 bits from the right)
    *offset = instruction & 0xFFFF; // Extract offset (16 bits from the right)
    *jsec = instruction & 0x3FFFFFF; // Extract jump address (26 bits from the right)
}

/* Instruction Decode */
int instruction_decode(unsigned op, struct_controls *controls) {
    // Decode the opcode and set control signals accordingly
    switch (op) {
        case 0x00: // R-type
            controls->RegDst = 1; // Destination register is rd
            controls->Jump = 0; // No jump
            controls->Branch = 0; // No branch
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 0; // No memory-to-register data
            controls->ALUOp = 2; // ALU operation: use funct field
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 0; // ALU source: use second register
            controls->RegWrite = 1; // Write to register
            break;
        case 0x02: // Jump
            controls->RegDst = 0; // No register destination
            controls->Jump = 1; // Jump instruction
            controls->Branch = 0; // No branch
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 0; // No memory-to-register data
            controls->ALUOp = 0; // ALU operation: don't care
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 0; // ALU source: don't care
            controls->RegWrite = 1; // Write to register (used for jump target)
            break;
        case 0x04: // Branch Equal
            controls->RegDst = 2; // Don't care
            controls->Jump = 0; // No jump
            controls->Branch = 1; // Branch instruction
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 2; // Data from ALU (for branching)
            controls->ALUOp = 1; // ALU operation: set to compare for equality
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 0; // ALU source: use second register
            controls->RegWrite = 0; // No register write
            break;
        case 0x08: // Add Immediate (addi)
            controls->RegDst = 0; // Destination register is rt
            controls->Jump = 0; // No jump
            controls->Branch = 0; // No branch
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 0; // No memory-to-register data
            controls->ALUOp = 0; // ALU operation: addition
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 1; // ALU source: immediate value
            controls->RegWrite = 1; // Write to register
            break;
        case 0x0A: // Set Less Than Immediate (slti)
            controls->RegDst = 1; // Destination register is rt
            controls->Jump = 0; // No jump
            controls->Branch = 0; // No branch
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 0; // No memory-to-register data
            controls->ALUOp = 2; // ALU operation: set less than (signed)
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 0; // ALU source: use second register
            controls->RegWrite = 1; // Write to register
            break;
        case 0x0B: // Set Less Than Immediate (sltiu)
            controls->RegDst = 1; // Destination register is rt
            controls->Jump = 0; // No jump
            controls->Branch = 0; // No branch
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 0; // No memory-to-register data
            controls->ALUOp = 3; // ALU operation: set less than (unsigned)
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 0; // ALU source: use second register
            controls->RegWrite = 1; // Write to register
            break;
        case 0xF: // Load Upper Immediate (lui)
            controls->RegDst = 0; // Destination register is rt
            controls->Jump = 0; // No jump
            controls->Branch = 0; // No branch
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 0; // No memory-to-register data
            controls->ALUOp = 6; // ALU operation: load upper immediate
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 1; // ALU source: immediate value
            controls->RegWrite = 1; // Write to register
            break;
        case 0x23: // Load Word (lw)
            controls->RegDst = 0; // Destination register is rt
            controls->Jump = 0; // No jump
            controls->Branch = 0; // No branch
            controls->MemRead = 1; // Read from memory
            controls->MemtoReg = 1; // Data comes from memory
            controls->ALUOp = 0; // ALU operation: addition
            controls->MemWrite = 0; // No memory write
            controls->ALUSrc = 1; // ALU source: immediate value
            controls->RegWrite = 1; // Write to register
            break;
        case 0x2B: // Store Word (sw)
            controls->RegDst = 2; // Don't care
            controls->Jump = 0; // No jump
            controls->Branch = 0; // No branch
            controls->MemRead = 0; // No memory read
            controls->MemtoReg = 2; // Data comes from ALU
            controls->ALUOp = 0; // ALU operation: addition
            controls->MemWrite = 1; // Write to memory
            controls->ALUSrc = 1; // ALU source: immediate value
            controls->RegWrite = 0; // No register write
            break;
        default:
            // Halt condition: illegal opcode
            return 1;
    }
    return 0; // No halt condition
}

/* Read Register */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
    // Read data from registers specified by r1 and r2
    *data1 = Reg[r1]; // Load value from register r1 into data1
    *data2 = Reg[r2]; // Load value from register r2 into data2
}

/* Sign Extend */
void sign_extend(unsigned offset, unsigned *extended_value) {
    // Extend the 16-bit offset to a 32-bit signed value
    *extended_value = (offset & 0x8000) ? (offset | 0xFFFF0000) : (offset & 0x0000FFFF);
    // If the sign bit is set, extend with 1s, else keep as 16-bit unsigned
}

/* ALU Operations */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero) {
    // Select ALU source based on ALUSrc control signal
    data2 = ALUSrc ? extended_value : data2; // If ALUSrc is 1, use extended value, otherwise use data2
    if (ALUOp == 2) { // Special case for R-type instructions
        // Determine ALU operation based on funct code
        switch (funct) {
            case 0x20: ALUOp = 0; break; // Addition
            case 0x22: ALUOp = 1; break; // Subtraction
            case 0x24: ALUOp = 4; break; // AND
            case 0x25: ALUOp = 5; break; // OR
            case 0x2A: ALUOp = 2; break; // Set Less Than (signed)
            case 0x2B: ALUOp = 3; break; // Set Less Than (unsigned)
            default:
                // Invalid funct code, halt
                return 1;
        }
        ALU(data1, data2, ALUOp, ALUresult, Zero); // Perform ALU operation
    } else {
        ALU(data1, data2, ALUOp, ALUresult, Zero); // Perform ALU operation
    }
    return 0; // No halt condition
}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {
    // Handle memory read or write operations based on control signals
    if (MemRead) {
        // Check if ALUresult is word-aligned and within bounds
        if (ALUresult % 4 != 0 || ALUresult >= 0xFFFF) {
            return 1; // Halt condition: address is not word-aligned or out of bounds
        }
        *memdata = Mem[ALUresult >> 2]; // Read from memory (convert address to index by dividing by 4)
    }
    if (MemWrite) {
        // Check if ALUresult is word-aligned and within bounds
        if (ALUresult % 4 != 0 || ALUresult >= 0xFFFF) {
            return 1; // Halt condition: address is not word-aligned or out of bounds
        }
        Mem[ALUresult >> 2] = data2; // Write to memory (convert address to index by dividing by 4)
    }
    return 0; // No halt condition
}

/* Write Register */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg) {
    // Write data to the specified register based on control signals
    if (RegWrite) {
        // Select data to write based on MemtoReg control signal
        unsigned writeData = MemtoReg ? memdata : ALUresult; // If MemtoReg is 1, use data from memory; otherwise use ALU result
        unsigned writeRegister = RegDst ? r3 : r2; // If RegDst is 1, write to register r3; otherwise write to register r2
        Reg[writeRegister] = writeData; // Write data to the register
    }
}

/* PC Update */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
    // Update the program counter based on control signals
    *PC += 4; // Increment PC by 4 (next instruction)
    if (Branch && Zero) {
        *PC += (extended_value << 2); // If branch is taken and Zero flag is set, update PC to branch target
    }
    if (Jump) {
        *PC = (jsec << 2) | (*PC & 0xF0000000); // Update PC to jump target while preserving upper bits
    }
}
