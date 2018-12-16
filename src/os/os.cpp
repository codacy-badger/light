#include "os/os.hpp"

OS* OS::windows = new Windows();
OS* OS::linux = new Linux();
OS* OS::mac = new Mac();
