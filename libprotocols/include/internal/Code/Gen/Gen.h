/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROS_INTERNAL_CODE_GEN_GEN_H
#define PROS_INTERNAL_CODE_GEN_GEN_H

#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

#include <internal/Code/AST/AST.h>

prosVector prosGen_genFunction(prosASTNode *funcNode);

PROTOCOLS_EXTERNC_END

#endif  // PROS_INTERNAL_CODE_GEN_GEN_H
