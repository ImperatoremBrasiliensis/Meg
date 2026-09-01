/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Source.h>
#include <internal/Code/SourceLocation.h>
#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef enum prosOutputReportType_e : uint8_t {
	PROS_OUTPUT_UNDEFINED,
	PROS_OUTPUT_FATAL,
	PROS_OUTPUT_ERROR,
	PROS_OUTPUT_WARNING,
	PROS_OUTPUT_INFO,
	PROS_OUTPUT_NOTE
} prosOutputReportType;

void prosOutput_report(
	prosOutputReportType type,
	prosSourceLocation loc,
	prosString label,
	prosString description,
	...
);

void prosOutput_reportError(
	prosSourceLocation loc,
	prosString label,
	prosString description,
	...
);

void prosOutput_reportWarning(
	prosSourceLocation loc,
	prosString label,
	prosString description,
	...
);

void prosOutput_reportInformation(
	prosSourceLocation loc,
	prosString label,
	prosString description,
	...
);

void prosOutput_reportNote(
	prosSourceLocation loc,
	prosString label,
	prosString description,
	...
);

PROTOCOLS_EXTERNC_END
