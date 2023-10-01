#pragma once

namespace ES::CommonTools {
    inline size_t charArraySize(const char* c) {
        size_t size = 0;
        for(size_t i = 0; c[i] != '\0'; i++) {
            size++;
        }
        return size;
    }
}