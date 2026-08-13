list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_library(libprotocols SHARED)
set_target_properties(libprotocols PROPERTIES OUTPUT_NAME "protocols")

target_sources(
   libprotocols
   PUBLIC
   FILE_SET HEADERS BASE_DIRS include
      FILES
         include/protocols/Orbita.h
         include/protocols/Event.h
         include/protocols/Utilities.h
         include/protocols/VirtualMachine.h
   PRIVATE
   include/internal/Orbita.h
   include/internal/Output.h
   include/internal/Utilities.h
   include/internal/VirtualMachine.h
   include/internal/Code/Bytecode.h
   include/internal/Code/Source.h
   include/internal/Code/SourceLocation.h
   include/internal/Code/AST/AST.h
   include/internal/Code/AST/ASTContext.h
   include/internal/Code/AST/ASTBuilder.h
   include/internal/Code/Lexicon/Token.h
   include/internal/Code/Lexicon/Lexer.h
   include/internal/Code/Lexicon/IdTable.h
   include/internal/Code/Parse/Parser.h
   include/internal/Code/Semantic/Scope.h
   include/internal/Code/Semantic/Semantics.h
   include/internal/Code/Analysis/CFG.h
   src/Orbita.c
   src/Output.c
   src/Utilities.c
   src/VirtualMachine.c
   src/Code/Bytecode.c
   src/Code/Source.c
   src/Code/AST/AST.c
   src/Code/AST/ASTContext.c
   src/Code/AST/ASTBuilder.c
   src/Code/Lexicon/Token.c
   src/Code/Lexicon/Lexer.c
   src/Code/Lexicon/IdTable.c
   src/Code/Parse/Parser.c
   src/Code/Semantic/Scope.c
   src/Code/Semantic/Semantics.c
   src/Code/Analysis/CFG.c
)

add_subdirectory(unitests)
