#include "virtual_machine.hpp"

#include <cassert>
#include <sstream>
#include <format>
#include <iostream>
#include <iomanip>
#include <span>

std::string toString(Opcode opcode)
{
    switch(opcode) {
    case ADD: return "ADD";
    case APTR: return "APTR";
    case CMP: return "CMP";
    case HALT: return "HALT";
    case JEZ: return "JEZ";
    case JNZ: return "JNZ";
    case MV: return "MV";
    case MV32: return "MV32";
    case MVI: return "MVI";
    case MVI32: return "MVI32";
    case OUT: return "OUT";
    case SUB: return "SUB";
    case XOR: return "XOR";
    case INVALID:
    default:
        return "INVALID";
    }
}

std::string toString(Register8 reg)
{
    switch(reg)
    {
    case a: return "a";
    case b: return "b";
    case c: return "c";
    case d: return "d";
    case e: return "e";
    case f: return "f";
    case ptr_c: return "(ptr+c)";
    case inv8:
    default:
        return "invalid";
    }
}

std::string toString(Register32 reg)
{
    switch(reg)
    {
    case la: return "la";
    case lb: return "lb";
    case lc: return "lc";
    case ld: return "ld";
    case ptr: return "ptr";
    case pc: return "pc";
    case inv32:
    default: return "invalid";
    }
}

uint8_t getDDD(uint8_t byte)
{
    return static_cast<uint8_t>((static_cast<unsigned int>(byte) & DDD_Mask) >> 3);
}

uint8_t getSSS(uint8_t byte)
{
    return static_cast<uint8_t>((static_cast<unsigned int>(byte) & SSS_Mask));
}

Register8 getDDD8(uint8_t byte)
{
    return static_cast<Register8>(getDDD(byte));
}

Register32 getDDD32(uint8_t byte)
{
    return static_cast<Register32>(getDDD(byte));
}

Register8 getSSS8(uint8_t byte)
{
    return static_cast<Register8>(getSSS(byte));
}

Register32 getSSS32(uint8_t byte)
{
    return static_cast<Register32>(getSSS(byte));
}

uint32_t read(std::span<const uint8_t> data)
{
    assert(data.size() >= 4);
    return data[0]
        + (data[1] << 8)
        + (data[2] << 16)
        + (data[3] << 24);
}

Opcode getOpcode(uint8_t byte)
{
    if(byte == 0xC2) {
        return ADD;
    } else if(byte == 0xE1) {
        return APTR;
    } else if(byte == 0xC1) {
        return CMP;
    } else if(byte == 0x01) {
        return HALT;
    } else if(byte == 0x21) {
        return JEZ;
    } else if(byte == 0x22) {
        return JNZ;
    } else if(byte == 0x02) {
        return OUT;
    } else if(byte == 0xC3) {
        return SUB;
    } else if(byte == 0xC4) {
        return XOR;
    } else if((byte & MV_Mask) > 0) {
        if(getSSS(byte) == 0) {
            return MVI;
        }
        return MV;
    } else if((byte & MV32_Mask) > 0) {
        if(getSSS(byte) == 0) {
            return MVI32;
        }
        return MV32;
    }
    return INVALID;
}

std::size_t instructionSize(Opcode opcode)
{
    switch(opcode) {
    case ADD:
    case CMP:
    case HALT:
    case MV:
    case MV32:
    case OUT:
    case SUB:
    case XOR:
        return 1;
    case APTR:
    case MVI:
        return 2;
    case JEZ:
    case JNZ:
    case MVI32:
        return 5;

    case INVALID:
    default:
        return 0;
    }
}

std::vector<uint8_t> instruction_decode(const std::vector<uint8_t> &inVec)
{
    VirtualMachine m(inVec);
    try {

        auto result = m.run();

        if(m.wasSuccessful()) {
            std::cout << "\n\nSuccess\n\n";
            return result;
        } else {
            std::cout << "Failed";
            return {};
        }
    } catch(const std::exception &ex) {
        std::cout << "Exception caught. Reason: " << ex.what() << std::endl;
        auto r = m.result();
        std::cout << "\nResult so far: \n " << std::string(r.begin(), r.end()) << std::endl;
    }

    return {};
}

VirtualMachine::VirtualMachine(std::vector<uint8_t> memory)
    : m_memory(std::move(memory))
{
    for(size_t i = 0; i < 7; ++i) {
        m_registers8[i] = 0;
        m_registers32[i] = 0;
    }
}

std::vector<uint8_t> VirtualMachine::run()
{
    m_result.clear();
    m_result.reserve(m_memory.size());
    m_stop = false;
    m_success = false;

    while(pc() < m_memory.size() && !m_stop) {
        uint8_t byte = m_memory[pc()];
        Opcode code = getOpcode(byte);
        std::size_t len = instructionSize(code);
        auto instruction = std::span(&m_memory[pc()], len);

        advancePc(len);

        executeInstruction(code, len, instruction);
    }

    return m_result;
}

void VirtualMachine::executeInstruction(Opcode opcode, uint32_t len, std::span<const uint8_t> instruction)
{
    printInstruction(opcode, len, instruction);

    switch(opcode) {
    case ADD:
    {
        assert(instruction.size() == 1);
        m_registers8[a] = (m_registers8[a] + m_registers8[b]) % 256;
        std::cout << "a <- b";
        break;
    }
    case CMP:
    {
        assert(instruction.size() == 1);
        bool eq = (m_registers8[a] == m_registers8[b]);
        if(eq) {
            m_registers8[f] = 0;
        } else {
            m_registers8[f] = 1;
        }
        break;
    }
    case HALT:
    {
        assert(instruction.size() == 1);
        m_stop = true;
        m_success = true;
        break;
    }
    case MV:
    {
        assert(instruction.size() == 1);
        auto dest = getDDD8(instruction[0]);
        auto src = getSSS8(instruction[0]);

        if(dest == Register8::ptr_c) {
            auto address = m_registers32[Register32::ptr] + m_registers8[Register8::c];
            m_memory[address] = m_registers8[src];
        } else if(src == Register8::ptr_c) {
            auto address = m_registers32[Register32::ptr] + m_registers8[Register8::c];
            m_registers8[dest] = m_memory[address];
        } else {
            m_registers8[dest] = m_registers8[src];
        }
        std::cout << toString(dest) << " <- " << toString(src);
        break;
    }
    case MV32:
    {
        assert(instruction.size() == 1);
        auto dest = getDDD32(instruction[0]);
        auto src = getSSS32(instruction[0]);
        m_registers32[dest] = m_registers32[src];
        std::cout << toString(dest) << " <- " << toString(src);
        break;
    }
    case OUT:
    {
        assert(instruction.size() == 1);
        m_result.push_back(m_registers8[a]);
        std::cout << "a";
        break;
    }
    case SUB:
    {
        assert(instruction.size() == 1);
        if(m_registers8[b] > m_registers8[a]) {
            uint32_t res = 256u + m_registers8[a] - m_registers8[b];

            m_registers8[a] = static_cast<uint8_t>(res % 256);
        } else {
            m_registers8[a] = (m_registers8[a] - m_registers8[b]);
        }
        std::cout << "a <- b";
        break;
    }
    case XOR:
    {
        assert(instruction.size() == 1);
        m_registers8[a] = m_registers8[a] ^ m_registers8[b];
        std::cout << "a <- b";
        break;
    }
    case APTR:
    {
        assert(instruction.size() == 2);
        uint32_t value = m_registers32[ptr] + instruction[1];
        m_registers32[ptr] = value;
        std::cout << std::format("0x{:08x}", instruction[1]);
        break;
    }
    case MVI:
    {
        assert(instruction.size() == 2);
        auto dest = getDDD8(instruction[0]);

        if(dest == Register8::ptr_c) {
            auto address = m_registers32[Register32::ptr] + m_registers8[Register8::c];
            m_memory[address] = instruction[1];
        } else {
            m_registers8[dest] = instruction[1];
        }
        std::cout << toString(dest) << " <- " << std::dec << static_cast<int>(instruction[1]);
        break;
    }
    case JEZ:
    {
        assert(instruction.size() == 5);
        if(m_registers8[f] == 0) {
            uint32_t newAddress = read(std::span(&instruction[1], 4));
            setPc(newAddress);
            std::cout << std::format("0x{:08x}", newAddress);
        }
        break;
    }
    case JNZ:
    {
        assert(instruction.size() == 5);
        if(m_registers8[f] != 0) {
            uint32_t newAddress = read(std::span(&instruction[1], 4));
            setPc(newAddress);
            std::cout << std::format("0x{:08x}", newAddress);
        }
        break;
    }
    case MVI32:
    {
        assert(instruction.size() == 5);
        auto dest = getDDD32(instruction[0]);
        uint32_t val = read(std::span(&instruction[1], 4));
        m_registers32[dest] = val;
        std::cout << toString(dest) << " <- " << std::format("0x{:08x}", val);
        break;
    }
    case INVALID:
    default:
        throw std::runtime_error("Invalid Opcode!");
    }
}

uint32_t VirtualMachine::pc() const
{
    return m_registers32[Register32::pc];
}

void VirtualMachine::advancePc(uint32_t bytes)
{
    m_registers32[Register32::pc] += bytes;
}

void VirtualMachine::setPc(uint32_t address)
{
    m_registers32[Register32::pc] = address;
}

bool VirtualMachine::wasSuccessful() const
{
    return m_success;
}

void VirtualMachine::printInstruction(Opcode opcode, uint32_t len, std::span<const uint8_t> instruction)
{
    std::cout << "\n";
    for(size_t i = 0; i < len; ++i) {
        uint8_t b = instruction[i];
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)b << " ";
    }
    if(len == 1) {
        std::cout << "   ";
    }

    std::cout << " # " << toString(opcode) << " ";
}

std::vector<uint8_t> VirtualMachine::result()
{
    return m_result;
}