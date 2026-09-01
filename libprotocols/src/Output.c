/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */
#include <internal/Output.h>

#include <internal/Orbita.h>
#include <internal/Utilities.h>

#include <string.h>

void outputReport(
   prosOutputReportType type,
   prosSourceLocation loc,
   prosString label,
   prosString description,
   va_list va
) {
   prosString pfx;
   switch (type) {
   case PROS_OUTPUT_FATAL:
      pfx = "\033[1;31mFatal Error\033[0m";
      break;
   case PROS_OUTPUT_ERROR:
      pfx = "\033[1;31mError\033[0m";
      break;
   case PROS_OUTPUT_INFO:
      pfx = "\033[1;32mInfo\033[0m";
      break;
   case PROS_OUTPUT_WARNING:
      pfx = "\033[1;33mWarning\033[0m";
      break;
   case PROS_OUTPUT_NOTE:
      pfx = "\033[1;34mNote\033[0m";
      break;
   default:
      pfx = "\033[1mMessage\033[0m";
   }

   size_t len = snprintf(
      nullptr,
      0,
      "%s - %d, %d:\n",
      loc.src->filename,
      loc.line,
      loc.column
   );

   // Gets the filename.
   char file[len];
   snprintf(
      file,
      sizeof(file),
      "%s - %d, %d",
      loc.src->filename,
      loc.line,
      loc.column
   );
   file[len - 1] = '\0';

   // General-use buffer.
   static char buffer[1024];

   // Formats the label.
   len = vsnprintf(buffer, sizeof(buffer), label, va);
   buffer[len] = '\0';

   pros_print(
      1,
      pfx,
      "$ | $\n",
      file,
      buffer
   );

   if (loc.len < 128) {
      if (!loc.column) {
         pros_panic(
            "prosOutput_report():"
            " Column must not be zero."
            " note: In `loc` parameter."
         );
      }
      loc.column--;

      prosSource_load(loc.src, loc.vm);
      while (loc.len) {
         prosString p = &loc.src->content[loc.offset];

         size_t lnsz = 0;
         while (lnsz < loc.src->size - loc.offset) {
            if (p[lnsz] == '\n')
               break;

            lnsz++;
         }

         size_t redsz = loc.len < lnsz - loc.column ?
            loc.len :
            lnsz - loc.column;

         printf(
            " %6u | "
            "%.*s"
            "\033[4;31m%.*s\033[0m"  // Red area.
            "%.*s\n",
            loc.line,
            loc.column,
            p,
            (int) redsz,
            &p[loc.column],
            (int) (lnsz - (loc.column + redsz)),
            &p[loc.column + redsz]
         );

         loc.column = 0;
         loc.line++;
         loc.len -= redsz;
         loc.offset += lnsz + 1;
      }

      prosSource_unload(loc.src, loc.vm);
   }

   if (description)
      pros_print(1, nullptr, "        + $", description);
}

void prosOutput_report(
   prosOutputReportType type,
   prosSourceLocation loc,
   prosString label,
   prosString description,
   ...
) {
   va_list va;
   va_start(va);
   outputReport(type, loc, label, description, va);
   va_end(va);
}

void prosOutput_reportError(
   prosSourceLocation loc,
   prosString label,
   prosString description,
   ...
) {
   va_list va;
   va_start(va);
   outputReport(PROS_OUTPUT_ERROR, loc, label, description, va);
   va_end(va);
}

void prosOutput_reportWarning(
   prosSourceLocation loc [[maybe_unused]],
   prosString label [[maybe_unused]],
   prosString description [[maybe_unused]],
   ...
) {
   va_list va;
   va_start(va);
   outputReport(PROS_OUTPUT_WARNING, loc, label, description, va);
   va_end(va);
}

void prosOutput_reportInformation(
   prosSourceLocation loc,
   prosString label,
   prosString description,
   ...
) {
   va_list va;
   va_start(va);
   outputReport(PROS_OUTPUT_INFO, loc, label, description, va);
   va_end(va);
}

void prosOutput_reportNote(
   prosSourceLocation loc,
   prosString label,
   prosString description,
   ...
) {
   va_list va;
   va_start(va);
   outputReport(PROS_OUTPUT_NOTE, loc, label, description, va);
   va_end(va);
}
