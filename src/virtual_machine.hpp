#ifndef TOMSDATAONION_VIRTUAL_MACHINE_HPP
#define TOMSDATAONION_VIRTUAL_MACHINE_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <span>

#define DDD_Mask 0b00111000
#define SSS_Mask 0b00000111

#define MV_Mask     0b01000000
#define MV32_Mask   0b10000000

enum Opcode : uint8_t
{
    INVALID = 0x00,
    HALT = 0x01,
    OUT = 0x02,
    JEZ = 0x21,
    JNZ = 0x22,
    MV = 0b01111111,
    MVI = 0b01111000,
    MV32 = 0b10111111,
    MVI32 = 0b10111000,
    CMP = 0xC1,
    ADD = 0xC2,
    SUB = 0xC3,
    XOR = 0xC4,
    APTR = 0xE1,
};

enum Register8 : uint8_t
{
    inv8 = 0,
    a = 1,
    b = 2,
    c = 3,
    d = 4,
    e = 5,
    f = 6,
    ptr_c = 7,
};

enum Register32 : uint8_t
{
    inv32 = 0,
    la = 1,
    lb = 2,
    lc = 3,
    ld = 4,
    ptr = 5,
    pc = 6,
};

std::string toString(Opcode opcode);
std::string toString(Register8 reg);
std::string toString(Register32 reg);

uint8_t getDDD(uint8_t byte);
uint8_t getSSS(uint8_t byte);

Register8 getDDD8(uint8_t byte);
Register32 getDDD32(uint8_t byte);
Register8 getSSS8(uint8_t byte);
Register32 getSSS32(uint8_t byte);

uint32_t read(std::span<const uint8_t> data);

Opcode getOpcode(uint8_t byte);
std::size_t instructionSize(Opcode opcode);

std::vector<uint8_t> instruction_decode(const std::vector<uint8_t> &inVec);

class VirtualMachine
{
public:
    VirtualMachine(std::vector<uint8_t> memory);

    std::vector<uint8_t> run();
    void executeInstruction(Opcode opcode, uint32_t len, std::span<const uint8_t> instruction);

    uint32_t pc() const;
    void advancePc(uint32_t bytes);
    void setPc(uint32_t address);

    bool wasSuccessful() const;

    void printInstruction(Opcode opcode, uint32_t len, std::span<const uint8_t> instruction);

    std::vector<uint8_t> result();

private:
    uint8_t m_registers8[8];
    uint32_t m_registers32[7];
    std::vector<uint8_t> m_memory;

    bool m_stop;
    std::vector<uint8_t> m_result;
    bool m_success;
};

#endif // TOMSDATAONION_VIRTUAL_MACHINE_HPP