/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Source.h>

#include <internal/Orbita.h>
#include <internal/Utilities.h>
#include <internal/VirtualMachine.h>

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

prosSource prosSource_new(prosString filename, prosVirtualMachine *vm, bool load) {
	if (!filename)
		pros_panic("prosSource_new(): `filename` parameter is `nullptr`.");
	if (!vm)
		pros_panic("prosSource_new(): `vm` parameter is invalid.");

	prosSource ret = {
		.filename = filename
	};

	if (load)
		prosSource_load(&ret, vm);

	return ret;
}

void prosSource_del(prosSource *self) {
	if (!self)
		pros_panic("prosSource_del(): `self` parameter is invalid.");

	if (self->bytecode)
		prosBytecode_del(&self->bytecode);
	if (self->content)
		free((char *) self->content);
	if (self->filePath)
		free(self->filePath);

	*self = (prosSource){};
}

bool prosSource_load(prosSource *self, prosVirtualMachine *vm) {
	if (!self)
		pros_panic("prosSource_del(): `self` parameter is invalid.");
	if (!vm)
		pros_panic("prosSource_del(): `vm` parameter is invalid.");

	if (self->content) {
		self->refCount++;
		return true;
	}

	if (!self->filePath) {
		size_t len = snprintf(nullptr, 0, "%s/%s", (*vm)->cwd, self->filename) + 1;
		self->filePath = malloc(len);
		snprintf(self->filePath, len, "%s/%s", (*vm)->cwd, self->filename);
	}

	struct stat fileInfo;
	if (stat(self->filePath, &fileInfo)) {
		(*vm)->pushError(
			vm,
			"Could not access file the file '$', $.",
			self->filePath,
			(*vm)->cwd
		);
		goto clearPath;
	}

	FILE *file = fopen(self->filePath, "r");
	if (!file) {
		(*vm)->pushError(
			vm,
			"Could not access the file '$', $.",
			self->filePath,
			strerror(errno)
		);

		goto clearPath;
	}

	self->size = fileInfo.st_size;
	self->content = malloc(self->size);
	if (!self->content) {
		(*vm)->pushError(vm, "`malloc()` failed, $", strerror(errno));
		goto closeFile;
	}
	fread((void *) self->content, 1, self->size, file);

	fclose(file);
	self->refCount = 1;
	return true;

closeFile:
	fclose(file);

clearPath:
	self->filePath = nullptr;
	return false;
}

void prosSource_unload(prosSource *self, prosVirtualMachine *vm) {
	if (!self)
		pros_panic("prosSource_unload(): `self` parameter is invalid.");
	if (!vm)
		pros_panic("prosSource_unload(): `vm` parameter is invalid.");

	if (!self->refCount)
		return;

	if (self->refCount == 1) {
		if (self->content)
			free((void *) self->content);
		self->content = nullptr;
		self->size = 0;
	}
	self->refCount--;
}
