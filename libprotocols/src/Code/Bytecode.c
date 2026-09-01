/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Bytecode.h>

prosBytecode prosBytecode_new() {
	prosBytecode ret = malloc(sizeof(struct prosBytecode_s));
	ret = &(struct prosBytecode_s){};

	return ret;
}

void prosBytecode_del(prosBytecode *self) {
	free(self);
}
