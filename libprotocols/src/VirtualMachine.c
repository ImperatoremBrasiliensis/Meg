/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/VirtualMachine.h>
#include <protocols/VirtualMachine.h>

#include <internal/Orbita.h>
#include <internal/Utilities.h>

static int virtualMachinePushError(
	prosVirtualMachine *self,
	prosString msg,
	...
) {
	va_list va;
	va_start(va);

	static char buf[1024];
	pros_format(buf, sizeof(buf), msg, va);
	pros_print(
		1,
		"Runtime Error",
		"Error not treated:\n"
		"  | $\n"
		"Virtual Machine: $. Exiting...",
		buf,
		(*self)->name
	);

	exit(1);
}

prosVirtualMachine prosVirtualMachine_new(prosVirtualMachine_Config config) {
	prosVirtualMachine ret = malloc(sizeof(*ret));
	*ret = (struct prosVirtualMachine_s){
		.name = config.name,
		.cwd = config.workingDirectory,
		.pushError = virtualMachinePushError,
		.vmType = config.vmType
	};

	ret->sources = prosVector_new(sizeof(prosSource), nullptr);
	ret->mainSource = prosSource_new(config.mainFile, &ret, true);
	return ret;
}

void prosVirtualMachine_del(prosVirtualMachine *self) {
	prosVector_del(&(*self)->sources);
	prosSource_del(&(*self)->mainSource);
	**self = (struct prosVirtualMachine_s){};

	free(*self);
	*self = nullptr;
}

int prosVirtualMachine_run(prosVirtualMachine *self, bool ownProcess) {
	if (!self || !*self)
		pros_panic("prosVirtulMachine_run(): `self` parameter is invalid.");

	prosVirtualMachine vm = *self;

	if (vm->vmType != PROS_VIRTUAL_MACHINE_IMPERATIVE)
		return -1;

	if (ownProcess) {
		int pid = fork();
		if (pid == -1)
			return -1;
		if (pid)
			return 0;
	}

	return 0;
}
