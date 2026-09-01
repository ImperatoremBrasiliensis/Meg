/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Gen/Gen.h>

#include <internal/Code/LittleSenna/Bytecode.h>
#include <internal/Orbita.h>

#include <assert.h>

#define PROS_ALIGNUP(val, alignment) \
   ((val + (alignment - 1)) & ~(alignment - 1))

struct genContext {
   prosArena arena;
   size_t stackLevel;
   size_t maxStackLevel;
   struct genValue *valStack[1024];
   size_t valStackSize;
   struct {
      struct genValue *val;
      unsigned prior;
   } regs[16];
};

static size_t genContextStackAlloc(
   struct genContext *ctx,
   size_t size
) {
   if (size > 1) {  // Alignment.
      ctx->stackLevel = size <= PROS_DWORD ?
         PROS_ALIGNUP(ctx->stackLevel, size) :
         PROS_ALIGNUP(ctx->stackLevel, PROS_DWORD);
   }

   size_t offset = ctx->stackLevel;
   ctx->stackLevel += size;
   if (ctx->stackLevel > ctx->maxStackLevel) {
      ctx->maxStackLevel = ctx->stackLevel;
   }
   return offset;
}

struct genBasicBlock {
   size_t instrCount;
   prosInstruction instrs[1024];
   struct genBasicBlock *ou1, *out2;
   struct genTerminatorFormat {
      enum genBBTerminator {
         GEN_TER_RETURN,
         GEN_TER_BRANCH,
         GEN_TER_BRANCH_WITH_LINK,
         GEN_TER_CMP_REGISTER,
         GEN_TER_CMP_IMMEDIATE,
         GEN_TER_CMP_ZERO
      } type;
      union {
         struct {
            struct genBasicBlock *bra;
         } bra;
         struct {
            struct genBasicBlock *bra, *ret;
         } brl;
         struct {
            prosRegister reg1, reg2;
            struct genBasicBlock *then, *elseb;
         } cmpWithRegister;

         struct {
            prosRegister reg;
            prosImmediate8 ime;
            struct genBasicBlock *then, *elseb;
         } cmpWithImmediate;

         struct {
            prosRegister reg;
            struct genBasicBlock *then, *elseb;
         } cmpWithZero;
      } format;
   } terminator;

   bool closed;
};

static struct genBasicBlock *genBBOpen(struct genContext *ctx) {
   struct genBasicBlock *ret = prosArena_alloc(&ctx->arena, sizeof(*ret));
   *ret = (struct genBasicBlock){};

   return ret;
}

static void genBBAppend(struct genBasicBlock *bb, prosInstruction instr) {
   if (bb->instrCount == PROS_SIZEOF_ARRAY(bb->instrs)) {
      pros_panic("prosGen: Instruction overflow.");
   }

   bb->instrs[bb->instrCount++] = instr;
}

static void genBBClose(
   struct genBasicBlock *bb,
   struct genTerminatorFormat terminator
) {
   bb->closed = true;
   bb->terminator = terminator;
}

static void genBBStoreRegister(
   struct genBasicBlock *bb,
   prosRegister reg,
   size_t size,
   size_t offset
) {
   if (size > PROS_DWORD) {
      pros_panic("prosGen: Value size too large.");
   }

   prosInstruction instr = prosBytecode_newStr_SZ_S_I12_R(
      size,
      PROS_SPECIAL_REGISTER_SP,
      offset,
      reg
   );
   genBBAppend(bb, instr);
}

static void genBBLoadRegister(
   struct genBasicBlock *bb,
   prosRegister reg,
   size_t size,
   size_t offset
) {
   if (size > PROS_DWORD) {
      pros_panic("prosGen: Value too large.");
   }

   prosInstruction instr = prosBytecode_newLdr_SZ_S_I12_R(
      size,
      PROS_SPECIAL_REGISTER_SP,
      offset,
      reg
   );
   genBBAppend(bb, instr);
}

static void genBBMoveRegister(
   struct genBasicBlock *bb,
   prosRegister destReg,
   prosRegister srcReg
) {
   prosInstruction instr = prosBytecode_newMov_R1_R2(destReg, srcReg);
   genBBAppend(bb, instr);
}

static void genBBMoveImmediate(
   struct genBasicBlock *bb,
   prosRegister destReg,
   prosImmediate20 srcIme
) {
   prosInstruction instr = prosBytecode_newMov_R_I20(destReg, srcIme);
   genBBAppend(bb, instr);
}

enum genBBCalculationType {
   GEN_BB_CAL_ADD,
   GEN_BB_CAL_SUB,
   GEN_BB_CAL_MUL,
   GEN_BB_CAL_DIV
};

static void genBBCalculateRegister(
   struct genBasicBlock *bb,
   enum genBBCalculationType type,
   prosRegister destReg,
   prosRegister lhsReg,
   prosRegister rhsReg
) {
   prosInstruction instr;
   switch (type) {
   case GEN_BB_CAL_ADD:
      instr = prosBytecode_newAdd_R1_R2_R3(destReg, lhsReg, rhsReg);
      break;
   case GEN_BB_CAL_SUB:
      instr = prosBytecode_newSub_R1_R2_R3(destReg, lhsReg, rhsReg);
      break;
   case GEN_BB_CAL_MUL:
      instr = prosBytecode_newMul_R1_R2_R3(destReg, lhsReg, rhsReg);
      break;
   case GEN_BB_CAL_DIV:
      instr = prosBytecode_newDiv_R1_R2_R3(destReg, lhsReg, rhsReg);
      break;
   }

   genBBAppend(bb, instr);
}

static void genBBNotRegister(
   struct genBasicBlock *bb,
   prosRegister destReg,
   prosRegister srcReg
) {
   prosInstruction instr = prosBytecode_newNot_R1_R2(
      destReg,
      srcReg
   );

   genBBAppend(bb, instr);
}

struct genValue {
   prosASTNode *node;
   size_t size;
   enum genValueType {
      GEN_NONE = 0,
      GEN_REGISTER,
      GEN_STACK,
      GEN_CONST
   } type;
   union {
      prosRegister reg;
      size_t stackOffset;
      uint64_t value;
   } data;
};

static void genContextSpillRegister(
   struct genContext *ctx,
   struct genBasicBlock *bb,
   prosRegister reg
) {
   struct genValue *val = ctx->regs[reg].val;
   if (val->type != GEN_REGISTER) {
      return;
   }

   size_t offset = genContextStackAlloc(ctx, val->size);
   genBBStoreRegister(bb, reg, val->size, offset);

   ctx->regs[val->data.reg].val = nullptr;
   val->type = GEN_STACK;
   val->data.stackOffset = offset;
}

static prosRegister genContextGetRegister(
   struct genContext *ctx,
   struct genBasicBlock *bb,
   bool temp
) {
   int beg = 0, end = 0;
   if (temp) {
      // Temporary use registers.
      beg = PROS_REGISTER_R06;
      end = PROS_REGISTER_R11;
   } else {
      // Persistent registers.
      beg = PROS_REGISTER_R12;
      end = PROS_REGISTER_R15;
   }

   prosRegister reg = 0;
   unsigned prior = 0;
   for (int i = beg; i < end + 1; i++) {
      auto regi = &ctx->regs[i];
      if (!regi->val) {
         reg = i;
         prior = -1u;
         continue;
      }

      if (regi->prior++ > prior) {
         reg = i;
         prior = regi->prior;
      }
   }

   if (ctx->regs[reg].val) {
      genContextSpillRegister(ctx, bb, reg);
   }
   ctx->regs[reg].prior = 0;
   return reg;
}

static struct genValue *genValuePush(
   struct genContext *ctx,
   prosASTNode *node,
   size_t size
) {
   struct genValue *val = prosArena_alloc(&ctx->arena, sizeof(*val));
   *val = (struct genValue){
      .node = node,
      .size = size
   };

   // Pushes it in the stack.
   if (ctx->valStackSize == PROS_SIZEOF_ARRAY(ctx->valStack)) {
      pros_panic("prosGen: Value stack overflow.");
   }

   ctx->valStack[ctx->valStackSize] = val;
   ctx->valStackSize++;
   return val;
}

static void genValuePop(struct genContext *ctx) {
   assert(ctx->valStackSize && "Value stack is empty.");

   ctx->valStackSize--;
   struct genValue *val = ctx->valStack[ctx->valStackSize];

   if (val->type == GEN_REGISTER) {
      ctx->regs[val->data.reg].val = nullptr;
   }
}

static prosRegister genValueLoad(
   struct genContext *ctx,
   struct genBasicBlock *bb,
   struct genValue *val
) {
   if (val->type == GEN_REGISTER) {
      ctx->regs[val->data.reg].prior = 0;
      return val->data.reg;
   }

   prosRegister reg = genContextGetRegister(ctx, bb, true);
   if (val->type == GEN_STACK) {
      genBBLoadRegister(bb, reg, val->size, val->data.stackOffset);
   }

   val->type = GEN_REGISTER;
   val->data.reg = reg;
   ctx->regs[reg].val = val;
   ctx->regs[reg].prior = 0;
   return reg;
}

static struct genValue *genGenExpression(
   struct genContext *ctx,
   struct genBasicBlock *bb,
   prosASTNode *node,
   struct genValue *val
) {
   switch (node->type) {
   case PROS_AST_EXPR_BINARY_OPERATOR: {
      auto binOp = &node->nodeAs.binOpExpr;
      auto typeDecl = prosAST_getTypeDecl(binOp->type);
      auto lhs = genValuePush(ctx, node, typeDecl.as.builtinp->memorySize);
      auto rhs = genValuePush(ctx, node, typeDecl.as.builtinp->memorySize);
      genGenExpression(ctx, bb, binOp->operand1, lhs);
      genGenExpression(ctx, bb, binOp->operand2, rhs);

      switch (binOp->binOpType) {
      case PROS_AST_BIN_OP_EQUAL: {
         genBBMoveRegister(bb, genValueLoad(ctx, bb, lhs), genValueLoad(ctx, bb, rhs));
         genBBMoveRegister(bb, genValueLoad(ctx, bb, val), genValueLoad(ctx, bb, lhs));
         break;
      }

         enum genBBCalculationType calType;
      case PROS_AST_BIN_OP_PLUS:
         calType = GEN_BB_CAL_ADD;
         goto calculate;
      case PROS_AST_BIN_OP_MINUS:
         calType = GEN_BB_CAL_SUB;
         goto calculate;
      case PROS_AST_BIN_OP_STAR:
         calType = GEN_BB_CAL_MUL;
         goto calculate;
      case PROS_AST_BIN_OP_SLASH:
         calType = GEN_BB_CAL_DIV;
calculate:
         genBBCalculateRegister(
            bb,
            calType,
            genValueLoad(ctx, bb, val),
            genValueLoad(ctx, bb, lhs),
            genValueLoad(ctx, bb, rhs)
         );
         break;
      case PROS_AST_BIN_OP_PLUSEQUAL:
         calType = GEN_BB_CAL_ADD;
         goto calculateAndAssign;
      case PROS_AST_BIN_OP_MINUSEQUAL:
         calType = GEN_BB_CAL_SUB;
         goto calculateAndAssign;
      case PROS_AST_BIN_OP_STAREQUAL:
         calType = GEN_BB_CAL_MUL;
         goto calculateAndAssign;
      case PROS_AST_BIN_OP_SLASHEQUAL:
         calType = GEN_BB_CAL_DIV;
calculateAndAssign:
         genBBCalculateRegister(
            bb,
            calType,
            genValueLoad(ctx, bb, lhs),
            genValueLoad(ctx, bb, lhs),
            genValueLoad(ctx, bb, rhs)
         );
         genBBMoveRegister(
            bb,
            genValueLoad(ctx, bb, val),
            genValueLoad(ctx, bb, lhs)
         );
         break;

      default:
         pros_panic("prosGen: Unknown binary operator type.");
      }

      genValuePop(ctx);
      genValuePop(ctx);
      return val;
   }

   case PROS_AST_EXPR_UNARY_OPERATOR: {
      auto unaOp = &node->nodeAs.unaOpExpr;
      auto typeDecl = prosAST_getTypeDecl(unaOp->type);
      auto ope = genValuePush(ctx, node, typeDecl.as.builtinp->memorySize);
      genGenExpression(ctx, bb, unaOp->operand, ope);

      switch (unaOp->unaOpType) {
      case PROS_AST_UNA_OP_PLUS:
         genBBMoveRegister(
            bb,
            genValueLoad(ctx, bb, val),
            genValueLoad(ctx, bb, ope)
         );
         break;

      case PROS_AST_UNA_OP_MINUS:
         genBBNotRegister(
            bb,
            genValueLoad(ctx, bb, val),
            genValueLoad(ctx, bb, ope)
         );
         genBBAppend(
            bb,
            prosBytecode_newAdd_R1_R2_I16(
               genValueLoad(ctx, bb, val),
               genValueLoad(ctx, bb, val),
               1
            )
         );
         break;

      case PROS_AST_UNA_OP_EXCLAMATION:
         genBBNotRegister(
            bb,
            genValueLoad(ctx, bb, val),
            genValueLoad(ctx, bb, ope)
         );
         break;

      default:
         pros_panic("prosGen: Unknown unary operator.");
      }

      genValuePop(ctx);
      return val;
   }

   case PROS_AST_EXPR_PARENS:
      return genGenExpression(ctx, bb, node->nodeAs.parensExpr.childExpr, val);

   case PROS_AST_EXPR_LITERAL: {
      auto lit = &node->nodeAs.literalExpr;
      genBBMoveImmediate(bb, genValueLoad(ctx, bb, val), lit->intialValue);
      return val;
   }

   case PROS_AST_EXPR_DECL_REF: {
      auto ref = &node->nodeAs.declRefExpr;
      for (size_t i = ctx->valStackSize; i-- > 0;) {
         if (ctx->valStack[i]->node == ref->decl) {
            genBBMoveRegister(
               bb,
               genValueLoad(ctx, bb, val),
               genValueLoad(ctx, bb, ctx->valStack[i])
            );
            return val;
         }
      }

      pros_panic("prosGen: Can't reference global objects or functions.");
   }

   default:
      pros_panic("prosGen: Unknown expression node.");
   }
}

static struct genBasicBlock *genGenOperation(
   struct genContext *ctx,
   prosASTNode *node
) {
   assert(node->type == PROS_AST_SCOPE_OPERATION);

   size_t valCount = 0;
   struct genBasicBlock *bb = genBBOpen(ctx);

   auto nnav = prosASTListNavigator_new(node->nodeAs.operation.body);
   prosASTNode *n = nullptr;
   while ((n = prosASTListNavigator_next(&nnav))) {
      switch (n->type) {
      case PROS_AST_DECL_VAL: {
         auto decl = n->nodeAs.valDecl;
         auto typeDecl = prosAST_getTypeDecl(decl.type);

         if (typeDecl.isBuiltin) {
            auto val = genValuePush(ctx, n, typeDecl.as.builtinp->memorySize);
            genGenExpression(ctx, bb, decl.value->nodeAs.expression.child, val);
         } else {
            pros_panic("prosGen: Custom types are unsupported.");
         }

         valCount++;
         break;
      }
      case PROS_AST_DECL_VAR: {
         auto decl = n->nodeAs.varDecl;
         auto typeDecl = prosAST_getTypeDecl(decl.type);

         if (typeDecl.isBuiltin) {
            auto val = genValuePush(ctx, n, typeDecl.as.builtinp->memorySize);
            genGenExpression(ctx, bb, decl.initializer->nodeAs.expression.child, val);
         } else {
            pros_panic("prosGen: Custom types are unsupported.");
         }

         valCount++;
         break;
      }

      case PROS_AST_EXPRESSION_STATEMENT: {
         auto stmt = n->nodeAs.expressionStatement;
         auto typeDecl = prosAST_getTypeDecl(stmt.type);

         if (typeDecl.isBuiltin) {
            auto val = genValuePush(ctx, n, typeDecl.as.builtinp->memorySize);
            genGenExpression(ctx, bb, stmt.child, val);
            genValuePop(ctx);  // ExpressionStatement are never used.
         } else {
            pros_panic("prosGen: Custom types are unsupported.");
         }

         break;
      }

      default:
         pros_panic("prosGen: Unknown statement.");
      }
   }

   while (valCount--) {
      genValuePop(ctx);
   }

   if (!bb->closed) {
      genBBClose(
         bb,
         (struct genTerminatorFormat){
            .type = GEN_TER_RETURN,
         }
      );
   }

   return bb;
}

struct genRelocation {
   int offset;
   enum genRelocationType {
      GEN_REL_RET
   } type;
};

static int genJointBasicBlock(
   struct genContext *ctx,
   prosVector *ret,
   prosVector *rela,
   struct genBasicBlock *bb
) {
   int offset = ret->size;
   prosVector_pushArray(ret, bb->instrs, bb->instrCount);

   switch (bb->terminator.type) {
   case GEN_TER_RETURN: {
      int instroff = ret->size;
      prosVector_pushBack(ret, &(prosInstruction){});

      prosVector_pushBack(
         rela,
         &(struct genRelocation){
            .offset = instroff
         }
      );
   } break;
   case GEN_TER_BRANCH: {
      int instroff = ret->size;
      prosInstruction instr = prosBytecode_newBra_I24(
         (prosImmediate24) instroff + 1
      );
      prosVector_pushBack(ret, &instr);

      genJointBasicBlock(ctx, ret, rela, bb);
   } break;
   case GEN_TER_BRANCH_WITH_LINK: {
      int instroff = ret->size;
      prosVector_pushBack(ret, &(prosInstruction){});

      genJointBasicBlock(ctx, ret, rela, bb->terminator.format.brl.ret);

      int braoff = ret->size;
      genJointBasicBlock(ctx, ret, rela, bb);

      prosInstruction *instr = prosVector_getAt(ret, instroff);
      *instr = prosBytecode_newBra_I24(braoff);
   } break;
   case GEN_TER_CMP_REGISTER:
   case GEN_TER_CMP_IMMEDIATE:
   case GEN_TER_CMP_ZERO: {
      pros_panic("prosGen: Comparisons are not available.");
   } break;
   default:
      pros_panic("prosGen: Inavalid terminator.");
   }

   return offset;
}

static void genRelocate(prosVector *ret, prosVector *rela) {
   for (size_t i = 0; i < rela->size; i++) {
      struct genRelocation *rel = prosVector_getAt(rela, i);

      switch (rel->type) {
      case GEN_REL_RET:
         /*
          * It's expected that this function was called
          * before the epilogue insertion.
          */
         prosInstruction *instr = prosVector_getAt(ret, rel->offset);
         size_t off = ret->size - rel->offset;
         if (off > 1) {
            *instr = prosBytecode_newBra_I24(ret->size - rel->offset);
         } else {
            prosVector_popBack(ret);
         }
         break;
      }
   }
}

prosVector prosGen_genFunction(prosASTNode *node) {
   struct genContext ctx = {
      .arena = prosArena_new(nullptr)
   };

   prosVector rela = prosVector_new(sizeof(struct genRelocation), nullptr);
   prosVector ret = prosVector_new(sizeof(prosInstruction), nullptr);

   auto entry = genGenOperation(&ctx, node->nodeAs.funcDecl.body);
   size_t stackSize = PROS_ALIGNUP(ctx.stackLevel, 16);

   // Creates function prologue.
   prosInstruction prologue[] = {
      prosBytecode_newSal_I24(stackSize)
   };
   prosVector_pushArray(&ret, prologue, PROS_SIZEOF_ARRAY(prologue));

   // Joints all blocks and rellocates them.
   genJointBasicBlock(&ctx, &ret, &rela, entry);
   genRelocate(&ret, &rela);

   // Creates function epilogue.
   prosInstruction epilogue[] = {
      prosBytecode_newSdl_I24(stackSize),
      //prosBytecode_newRet(),
      {}
   };
   prosVector_pushArray(&ret, epilogue, PROS_SIZEOF_ARRAY(epilogue));

   prosVector_del(&rela);
   prosArena_del(&ctx.arena);
   return ret;
}
