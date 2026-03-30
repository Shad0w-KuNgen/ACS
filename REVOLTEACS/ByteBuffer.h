#pragma once
#include "types.h"
#include <vector>
#include <string>
#include <cstring>

// 25xx Full Hook System - ByteBuffer
// Pearl Guard referansindan uyarlanmis temel buffer sinifi

class ByteBuffer {
public:
    ByteBuffer() : _rpos(0), _wpos(0) {
        _storage.reserve(64);
    }

    ByteBuffer(size_t reserve) : _rpos(0), _wpos(0) {
        _storage.reserve(reserve);
    }

    ByteBuffer(const uint8* data, size_t len) : _rpos(0), _wpos(0) {
        _storage.assign(data, data + len);
        _wpos = len;
    }

    void clear() {
        _storage.clear();
        _rpos = _wpos = 0;
    }

    size_t rpos() const { return _rpos; }
    size_t wpos() const { return _wpos; }
    size_t size() const { return _storage.size(); }

    const uint8* contents() const {
        if (_storage.empty()) return nullptr;
        return &_storage[0];
    }

    // --- Write operators ---
    ByteBuffer& operator<<(uint8 value) {
        append<uint8>(value);
        return *this;
    }

    ByteBuffer& operator<<(uint16 value) {
        append<uint16>(value);
        return *this;
    }

    ByteBuffer& operator<<(uint32 value) {
        append<uint32>(value);
        return *this;
    }

    ByteBuffer& operator<<(uint64 value) {
        append<uint64>(value);
        return *this;
    }

    ByteBuffer& operator<<(int8 value) {
        append<int8>(value);
        return *this;
    }

    ByteBuffer& operator<<(int16 value) {
        append<int16>(value);
        return *this;
    }

    ByteBuffer& operator<<(int32 value) {
        append<int32>(value);
        return *this;
    }

    ByteBuffer& operator<<(float value) {
        append<float>(value);
        return *this;
    }

    ByteBuffer& operator<<(const std::string& value) {
        uint16 len = (uint16)value.length();
        append<uint16>(len);
        if (len > 0) {
            append((const uint8*)value.c_str(), len);
        }
        return *this;
    }

    // --- Read operators ---
    ByteBuffer& operator>>(uint8& value) {
        value = read<uint8>();
        return *this;
    }

    ByteBuffer& operator>>(uint16& value) {
        value = read<uint16>();
        return *this;
    }

    ByteBuffer& operator>>(uint32& value) {
        value = read<uint32>();
        return *this;
    }

    ByteBuffer& operator>>(uint64& value) {
        value = read<uint64>();
        return *this;
    }

    ByteBuffer& operator>>(int8& value) {
        value = read<int8>();
        return *this;
    }

    ByteBuffer& operator>>(int16& value) {
        value = read<int16>();
        return *this;
    }

    ByteBuffer& operator>>(int32& value) {
        value = read<int32>();
        return *this;
    }

    ByteBuffer& operator>>(float& value) {
        value = read<float>();
        return *this;
    }

    ByteBuffer& operator>>(std::string& value) {
        uint16 len = read<uint16>();
        if (len > 0 && _rpos + len <= size()) {
            value.assign((const char*)&_storage[_rpos], len);
            _rpos += len;
        } else {
            value.clear();
        }
        return *this;
    }

protected:
    template<typename T>
    void append(T value) {
        size_t s = sizeof(T);
        if (_storage.size() < _wpos + s)
            _storage.resize(_wpos + s);
        memcpy(&_storage[_wpos], &value, s);
        _wpos += s;
    }

    void append(const uint8* src, size_t len) {
        if (_storage.size() < _wpos + len)
            _storage.resize(_wpos + len);
        memcpy(&_storage[_wpos], src, len);
        _wpos += len;
    }

    template<typename T>
    T read() {
        if (_rpos + sizeof(T) > size()) {
            // Buffer overflow guvenlik - crash onleme
            return T(0);
        }
        T value;
        memcpy(&value, &_storage[_rpos], sizeof(T));
        _rpos += sizeof(T);
        return value;
    }

    size_t _rpos, _wpos;
    std::vector<uint8> _storage;
};
