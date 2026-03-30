#pragma once
#include "ByteBuffer.h"

// 25xx Full Hook System - Packet Class
// ByteBuffer uzerine opcode destegi

class Packet : public ByteBuffer {
public:
    Packet() : ByteBuffer(), m_opcode(0) {}

    Packet(uint8 opcode) : ByteBuffer(), m_opcode(opcode) {}

    Packet(const uint8* data, size_t len) : ByteBuffer(data, len), m_opcode(0) {
        if (len > 0) {
            m_opcode = data[0];
            _rpos = 1; // opcode'u atla, data'dan oku
        }
    }

    void Initialize(uint8 opcode) {
        clear();
        m_opcode = opcode;
    }

    uint8 GetOpcode() const { return m_opcode; }
    void SetOpcode(uint8 opcode) { m_opcode = opcode; }

private:
    uint8 m_opcode;
};
