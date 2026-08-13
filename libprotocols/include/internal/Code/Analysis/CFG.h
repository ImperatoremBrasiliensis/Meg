/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_ANALYSIS_CFG_H
#define PROTOCOLS_INTERNAL_ANALYSIS_CFG_H

#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

#include <internal/Code/AST/AST.h>
#include <internal/Code/SourceLocation.h>

bool prosCFG_analyzeFunc(prosASTNode *funcNode, prosAllocator *altor, prosSourceLocation loc);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_ANALYSIS_CFG_H
