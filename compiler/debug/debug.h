//
// Created by wylan on 12/19/24.
//

#ifndef debug_h
#define debug_h

#include "../chunk/chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif //debug_h
