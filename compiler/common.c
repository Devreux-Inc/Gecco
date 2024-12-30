//
// Created by wylan on 12/29/2024.
//

#include "common.h"

bool isWindows() {
#ifdef OS_Windows
    return true;
#else
    return false;
#endif
}
