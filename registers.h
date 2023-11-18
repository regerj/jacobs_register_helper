#include <cassert>
#include <cstdint>

#define IMPLEMENT_REGISTER_16_GET(FIELD, START, END) \
    static_assert(START >= 0 && START < 16);\
    static_assert(END >= 0 && END < 16 && END >= START);\
    uint16_t get_##FIELD() {\
        uint16_t buffer = register_raw >> START;\
        return buffer & (0xFFFF >> (15 - (END - START)));\
    }

#define IMPLEMENT_REGISTER_16_SET(FIELD, START, END) \
    static_assert(START >= 0 && START < 16);\
    static_assert(END >= 0 && END < 16 && END >= START);\
    bool set_##FIELD(uint16_t value) {\
        if (value >= (1 << (END - START + 1))) {\
            return false;\
        }\
        uint16_t mask = static_cast<uint16_t>(~((0xFFFF >> (15 - (END - START))) << START));\
        register_raw &= mask;\
        value = value << START;\
        register_raw |= value;\
        return true;\
    }

#define IMPLEMENT_REGISTER_32_GET(FIELD, START, END) \
    static_assert(START >= 0 && START < 32);\
    static_assert(END >= 0 && END < 32 && END >= START);\
    uint16_t get_##FIELD() {\
        uint16_t buffer = register_raw >> START;\
        return buffer & (0xFFFF'FFFF >> (31 - (END - START)));\
    }

#define IMPLEMENT_REGISTER_32_SET(FIELD, START, END) \
    static_assert(START >= 0 && START < 32);\
    static_assert(END >= 0 && END < 32 && END >= START);\
    bool set_##FIELD(uint32_t value) {\
        if (value >= (1 << (END - START + 1))) {\
            return false;\
        }\
        uint32_t mask = static_cast<uint32_t>(~((0xFFFF'FFFF >> (31 - (END - START))) << START));\
        register_raw &= mask;\
        value = value << START;\
        register_raw |= value;\
        return true;\
    }

// MAGICAL FOR_EACH MACRO from https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define PARENS ()

#define EXPAND(...) EXPAND3(EXPAND3(__VA_ARGS__))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                    \
  __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, FIELD, START, END, ...)                         \
  macro(FIELD, START, END)                                                     \
  __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define DECLARE_REGISTER_16(NAME, ...) \
    class NAME {\
        public:\
            NAME() {}\
            FOR_EACH(IMPLEMENT_REGISTER_16_GET, __VA_ARGS__);\
            FOR_EACH(IMPLEMENT_REGISTER_16_SET, __VA_ARGS__);\
            uint16_t get_register_value() { return register_raw; };\
            void clear_register_value() { register_raw = 0x0; };\
        private:\
            uint16_t register_raw = 0x0;\
    };\

#define DECLARE_REGISTER_32(NAME, ...) \
    class NAME {\
        public:\
            NAME() {}\
            FOR_EACH(IMPLEMENT_REGISTER_32_GET, __VA_ARGS__);\
            FOR_EACH(IMPLEMENT_REGISTER_32_SET, __VA_ARGS__);\
            uint32_t get_register_value() { return register_raw; };\
        private:\
            uint32_t register_raw = 0x0;\
    };
