#include "arch/arch.hpp"

#include "platform.hpp"

Arch* Arch::x64 = new Arch_x64();

Arch* Arch::get_current_arch () {
    switch (os_get_arch()) {
        case ARCH_TYPE_X64:  return Arch::x64;
        default:                return NULL;
    }
}
