/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_LITTLESENNA_BYTECODE_H
#define PROTOCOLS_INTERNAL_CODE_LITTLESENNA_BYTECODE_H

#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef enum prosInstructionCode_e : uint8_t {
   PROS_INSTRUCTION_ZERO = 0,

   /* 1. Data Processing Instructions */
   /* 1.1 Mathematical Instructions */
   PROS_INSTRUCTION_ADD_REG1_REG2_REG3 = 1,
   PROS_INSTRUCTION_ADD_REG1_REG2_IME16 = 2,
   PROS_INSTRUCTION_SUB_REG1_REG2_REG3 = 3,
   PROS_INSTRUCTION_SUB_REG1_REG2_IME16 = 4,
   PROS_INSTRUCTION_MUL_REG1_REG2_REG3 = 5,
   PROS_INSTRUCTION_MUL_REG1_REG2_IME16 = 6,
   PROS_INSTRUCTION_DIV_REG1_REG2_REG3 = 7,
   PROS_INSTRUCTION_DIV_REG1_REG2_IME16 = 8,

   /* 1.2 Movement Instruction */
   PROS_INSTRUCTION_MOV_REG1_REG2 = 9,
   PROS_INSTRUCTION_MOV_REG_IME20 = 10,
   PROS_INSTRUCTION_MOV_E_K_REG_IME16_LSL = 11,
   PROS_INSTRUCTION_MOV_E_Z_REG_IME16_LSL = 12,

   /* 1.3 Logical Instructions */
   PROS_INSTRUCTION_AND_REG1_REG2_REG3 = 13,
   PROS_INSTRUCTION_LOR_REG1_REG2_REG3 = 14,
   PROS_INSTRUCTION_EOR_REG1_REG2_REG3 = 15,
   PROS_INSTRUCTION_NOT_REG1_REG2 = 16,

   /* 2. Control Flow Instructions */
   /* 2.1 Branch Instructions */
   PROS_INSTRUCTION_BRA_REG = 17,
   PROS_INSTRUCTION_BRA_IME24 = 18,
   PROS_INSTRUCTION_BRA_E_A_REG = 19,
   PROS_INSTRUCTION_BRL_REG = 20,
   PROS_INSTRUCTION_BRL_IME24 = 21,
   PROS_INSTRUCTION_BRL_E_A_REG = 22,
   PROS_INSTRUCTION_RET = 23,

   /* 2.2 Comparison Instructions */
   PROS_INSTRUCTION_EQL_IME16_REG1_REG2 = 24,
   PROS_INSTRUCTION_EQL_IME12_REG_IME8 = 25,
   PROS_INSTRUCTION_GTT_IME16_REG1_REG2 = 26,
   PROS_INSTRUCTION_GTT_IME12_REG_IME8 = 27,
   PROS_INSTRUCTION_LTT_IME16_REG1_REG2 = 28,
   PROS_INSTRUCTION_LTT_IME12_REG_IME8 = 29,
   PROS_INSTRUCTION_ISZ_IME20_REG = 30,

   /* 2.3 System Exception Generation */
   PROS_INSTRUCTION_SYS_IME24 = 31,

   /* 3. Data Management Instructions */
   /* 3.1 Memory Access Instructions */
   PROS_INSTRUCTION_STR_SIZE_REG1_IME12_REG2 = 32,
   PROS_INSTRUCTION_STR_SIZE_SREG_IME12_REG = 33,
   PROS_INSTRUCTION_STP_REG1_IME12_REG2_REG3 = 34,
   PROS_INSTRUCTION_STP_SREG_IME12_REG1_REG2 = 35,
   PROS_INSTRUCTION_LDR_SIZE_REG1_IME12_REG2 = 36,
   PROS_INSTRUCTION_LDR_SIZE_SREG_IME12_REG = 37,
   PROS_INSTRUCTION_LDP_REG1_IME12_REG2_REG3 = 38,
   PROS_INSTRUCTION_LDP_SREG_IME12_REG1_REG2 = 39,

   /* 4. Special Register Instructions */
   /* 4.1 Stack Allocation Instructions */
   PROS_INSTRUCTION_SAL_REG = 40,
   PROS_INSTRUCTION_SAL_IME24 = 41,
   PROS_INSTRUCTION_SDL_REG = 42,
   PROS_INSTRUCTION_SDL_IME24 = 43
} prosInstructionCode;

typedef _BitInt(4) prosImmediate4;
typedef _BitInt(8) prosImmediate8;
typedef _BitInt(12) prosImmediate12;
typedef _BitInt(16) prosImmediate16;
typedef _BitInt(20) prosImmediate20;
typedef _BitInt(24) prosImmediate24;

typedef unsigned _BitInt(4) prosLogicalShiftLeft;
static constexpr prosLogicalShiftLeft PROS_LSL_00 = 0;
static constexpr prosLogicalShiftLeft PROS_LSL_16 = 1;
static constexpr prosLogicalShiftLeft PROS_LSL_32 = 2;
static constexpr prosLogicalShiftLeft PROS_LSL_48 = 3;

typedef unsigned _BitInt(4) prosWordSize;
static constexpr prosWordSize PROS_BYTE = 1;
static constexpr prosWordSize PROS_HWORD = 2;
static constexpr prosWordSize PROS_WORD = 4;
static constexpr prosWordSize PROS_DWORD = 8;

typedef enum prosRegister_e : uint8_t {
   PROS_REGISTER_R00 = 0,
   PROS_REGISTER_R01 = 1,
   PROS_REGISTER_R02 = 2,
   PROS_REGISTER_R03 = 3,
   PROS_REGISTER_R04 = 4,
   PROS_REGISTER_R05 = 5,
   PROS_REGISTER_R06 = 6,
   PROS_REGISTER_R07 = 7,
   PROS_REGISTER_R08 = 8,
   PROS_REGISTER_R09 = 9,
   PROS_REGISTER_R10 = 10,
   PROS_REGISTER_R11 = 11,
   PROS_REGISTER_R12 = 12,
   PROS_REGISTER_R13 = 13,
   PROS_REGISTER_R14 = 14,
   PROS_REGISTER_R15 = 15,
} prosRegister;

typedef enum prosSpecialRegister_e : uint8_t {
   PROS_SPECIAL_REGISTER_PC = 0,
   PROS_SPECIAL_REGISTER_SP = 1,
   PROS_SPECIAL_REGISTER_BL = 2,
} prosSpecialRegister;

typedef struct PROS_PACKED prosInstruction_s {
   prosInstructionCode code;
   union PROS_PACKED prosIntrParamFormat {
      struct PROS_PACKED prosInstrAdd_R1_R2_R3 {
         prosRegister reg1:4, reg2:4, reg3:4;
      } add_r1_r2_r3;

      struct PROS_PACKED prosInstrAdd_R1_R2_I16 {
         prosRegister reg1:4, reg2:4;
         prosImmediate16 ime:16;
      } add_r1_r2_i16;

      struct PROS_PACKED prosInstrSub_R1_R2_R3 {
         prosRegister reg1:4, reg2:4, reg3:4;
      } sub_r1_r2_r3;

      struct PROS_PACKED prosInstrSub_R1_R2_I16 {
         prosRegister reg1:4, reg2:4;
         prosImmediate16 ime:16;
      } sub_r1_r2_i16;

      struct PROS_PACKED prosInstrMul_R1_R2_R3 {
         prosRegister reg1:4, reg2:4, reg3:4;
      } mul_r1_r2_r3;

      struct PROS_PACKED prosInstrMul_R1_R2_I16 {
         prosRegister reg1:4, reg2:4;
         prosImmediate16 ime:16;
      } mul_r1_r2_i16;

      struct PROS_PACKED prosInstrDiv_R1_R2_R3 {
         prosRegister reg1:4, reg2:4, reg3:4;
      } div_r1_r2_r3;

      struct PROS_PACKED prosInstrDiv_R1_R2_I16 {
         prosRegister reg1:4, reg2:4;
         prosImmediate16 ime:16;
      } div_r1_r2_i16;

      struct prosIntrMov_R_I20 {
         prosRegister reg1:4, reg2:4;
      } mov_r1_r2;

      struct PROS_PACKED prosInstrMov_R_I20 {
         prosRegister reg:4;
         prosImmediate20 ime:20;
      } mov_r_i20;

      struct PROS_PACKED prosInstrMovEk_R_I16_L {
         prosRegister reg:4;
         prosImmediate16 ime:16;
         prosLogicalShiftLeft lsl:4;
      } movEk_r_i12_l;

      struct PROS_PACKED prosInstrMov_Ez_R_I16_L {
         prosRegister reg:4;
         prosImmediate16 ime:16;
         prosLogicalShiftLeft lsl:4;
      } movEz_r_i12_l;

      struct PROS_PACKED prosInstrAnd_R1_R2_R3 {
         prosRegister reg1:4, reg2:4, reg3:4;
      } and_r1_r2_r3;

      struct PROS_PACKED prosInstrLor_R1_R2_R3 {
         prosRegister reg1:4, reg2:4, reg3:4;
      } lor_r1_r2_r3;

      struct PROS_PACKED prosInstrEor_R1_R2_R3 {
         prosRegister reg1:4, reg2:4, reg3:4;
      } eor_r1_r2_r3;

      struct PROS_PACKED prosInstrNot_R1_R2 {
         prosRegister reg1:4, reg2:4;
      } not_r1_r2;

      struct PROS_PACKED prosInstrBra_R {
         prosRegister reg:4;
      } bra_r;

      struct PROS_PACKED prosInstrBra_i24 {
         prosImmediate24 ime:24;
      } bra_i24;

      struct PROS_PACKED prosInstrBra_Ea_R {
         prosRegister reg:4;
      } braEa_r;

      struct PROS_PACKED prosInstrBrl_R {
         prosRegister reg:4;
      } brl_r;

      struct PROS_PACKED prosInstrBrl_i24 {
         prosImmediate24 ime:24;
      } brl_i24;

      struct PROS_PACKED prosInstrBrl_Ea_R {
         prosRegister reg:4;
      } brlEa_r;

      struct PROS_PACKED prosInstrEql_I16_R1_R2 {
         prosImmediate16 ime:16;
         prosRegister reg1:4, reg2:4;
      } eql_i16_r1_r2;

      struct PROS_PACKED prosInstrEql_I12_R_I8 {
         prosImmediate12 ime1:12;
         prosRegister reg:4;
         prosImmediate8 ime2:8;
      } eql_i12_r_i8;

      struct PROS_PACKED prosInstrGtt_I16_R1_R2 {
         prosImmediate12 ime:12;
         prosRegister reg1:4, reg2:4;
      } gtt_i16_r1_r2;

      struct PROS_PACKED prosInstrGtt_I12_R_I8 {
         prosImmediate12 ime1:12;
         prosRegister reg:4;
         prosImmediate8 ime2:8;
      } gtt_i12_r_i8;

      struct PROS_PACKED prosInstrLtt_I16_R1_R2 {
         prosImmediate16 ime:16;
         prosRegister reg1:4, reg2:4;
      } ltt_i16_r1_r2;

      struct PROS_PACKED prosInstrLtt_I12_R_I8 {
         prosImmediate12 ime1:12;
         prosRegister reg:4;
         prosImmediate8 ime2:8;
      } ltt_i12_r_i8;

      struct PROS_PACKED prosInstrIsz_I20_R {
         prosImmediate20 ime:20;
         prosRegister reg:4;
      } isz_ime20_r;

      struct PROS_PACKED prosInstrSys_I24 {
         prosImmediate24 ime:24;
      } sys_i24;

      struct PROS_PACKED prosInstrStr_SZ_R1_I12_R2 {
         prosWordSize size:4;
         prosRegister reg1:4;
         prosImmediate16 ime:12;
         prosRegister reg2:4;
      } str_sz_r1_i12_r2;

      struct PROS_PACKED prosInstrStr_SZ_S_I12_R {
         prosWordSize size:4;
         prosSpecialRegister sreg:4;
         prosImmediate16 ime:12;
         prosRegister reg:4;
      } str_sz_s_i12_r;

      struct PROS_PACKED prosInstrStp_R1_I12_R2_R3 {
         prosRegister reg1:4;
         prosImmediate16 ime:12;
         prosRegister reg2:4, reg3:4;
      } stp_r1_i12_r2_r3;

      struct PROS_PACKED prosInstrStp_S_I12_R1_R2 {
         prosSpecialRegister sreg:4;
         prosImmediate16 ime:12;
         prosRegister reg1:4, reg2:4;
      } stp_s_i12_r1_r2;

      struct PROS_PACKED prosInstrLdr_SZ_R1_I12_R2 {
         prosWordSize size:4;
         prosRegister reg1:4;
         prosImmediate16 ime:12;
         prosRegister reg2:4;
      } ldr_sz_r1_i12_r2;

      struct PROS_PACKED prosInstrLdr_SZ_S_I12_R {
         prosWordSize size:4;
         prosSpecialRegister sreg:4;
         prosImmediate16 ime:12;
         prosRegister reg:4;
      } ldr_sz_s_i12_r;

      struct PROS_PACKED prosInstrLdp_R1_I12_R2_R3 {
         prosRegister reg1:4;
         prosImmediate16 ime:12;
         prosRegister reg2:4, reg3:4;
      } ldp_r1_i12_r2_r3;

      struct PROS_PACKED prosInstrLdp_S_I12_R1_R2 {
         prosSpecialRegister sreg:4;
         prosImmediate16 ime:12;
         prosRegister reg1:4, reg2:4;
      } ldp_s_i12_r1_r2;

      struct PROS_PACKED prosInstrSal_R {
         prosRegister reg:4;
      } sal_r;

      struct PROS_PACKED prosInstrSal_I24 {
         prosImmediate24 ime:24;
      } sal_i24;

      struct PROS_PACKED prosInstrSdl_R {
         prosRegister reg:4;
      } sdl_r;

      struct PROS_PACKED prosInstrSdl_I24 {
         prosImmediate24 ime:24;
      } sdl_i24;
   } as;
} prosInstruction;

prosInstruction prosBytecode_newAdd_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newAdd_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme);
prosInstruction prosBytecode_newSub_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newSub_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme);
prosInstruction prosBytecode_newMul_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newMul_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme);
prosInstruction prosBytecode_newDiv_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newDiv_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme);

prosInstruction prosBytecode_newMov_R1_R2(prosRegister destReg, prosRegister srcReg);
prosInstruction prosBytecode_newMov_R_I20(prosRegister destReg, prosImmediate20 srcIme);
prosInstruction prosBytecode_newMovEk_R_I16_L(prosRegister destReg, prosImmediate16 srcIme, prosLogicalShiftLeft lsl);
prosInstruction prosBytecode_newMovEz_R_I16_L(prosRegister destReg, prosImmediate16 srcIme, prosLogicalShiftLeft lsl);

prosInstruction prosBytecode_newAnd_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newLor_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newEor_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newNot_R1_R2(prosRegister destReg, prosRegister srcReg);

prosInstruction prosBytecode_newBra_R(prosRegister offsetReg);
prosInstruction prosBytecode_newBra_I24(prosImmediate24 offsetIme);
prosInstruction prosBytecode_newBraEa_R(prosRegister adrReg);
prosInstruction prosBytecode_newBrl_R(prosRegister offsetReg);
prosInstruction prosBytecode_newBrl_I24(prosImmediate24 offsetIme);
prosInstruction prosBytecode_newBrlEa_R(prosRegister adrReg);
prosInstruction prosBytecode_newRet();

prosInstruction prosBytecode_newEql_I16_R1_R2(prosImmediate16 offsetIme, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newEql_I12_R_I8(prosImmediate12 offsetIme, prosRegister lhsReg, prosImmediate8 rhsIme);
prosInstruction prosBytecode_newGtt_I16_R1_R2(prosImmediate16 offsetIme, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newGtt_I12_R_I8(prosImmediate12 offsetIme, prosRegister lhsReg, prosImmediate8 rhsIme);
prosInstruction prosBytecode_newLtt_I16_R1_R2(prosImmediate16 offsetIme, prosRegister lhsReg, prosRegister rhsReg);
prosInstruction prosBytecode_newLtt_I12_R_I8(prosImmediate12 offsetIme, prosRegister lhsReg, prosImmediate8 rhsIme);
prosInstruction prosBytecode_newIsz_I20_R(prosImmediate20 offsetIme, prosRegister srcReg);

prosInstruction prosBytecode_newSys_I24(prosImmediate24);

prosInstruction prosBytecode_newStr_SZ_R1_I12_R2(prosWordSize size, prosRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg);
prosInstruction prosBytecode_newStr_SZ_S_I12_R(prosWordSize size, prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg);
prosInstruction prosBytecode_newStp_R1_I12_R2_R3(prosRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg1, prosRegister srcReg2);
prosInstruction prosBytecode_newStp_S_I12_R1_R2(prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg1, prosRegister srcReg2);
prosInstruction prosBytecode_newLdr_SZ_R1_I12_R2(prosWordSize size, prosRegister addrReg, prosImmediate12 offsetIme, prosRegister destReg);
prosInstruction prosBytecode_newLdr_SZ_S_I12_R(prosWordSize size, prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister destReg);
prosInstruction prosBytecode_newLdp_R1_I12_R2_R3(prosRegister addrReg, prosImmediate12 offsetIme, prosRegister destReg1, prosRegister destReg2);
prosInstruction prosBytecode_newLdp_S_I12_R1_R2(prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister destReg1, prosRegister destReg2);

prosInstruction prosBytecode_newSal_R(prosRegister srcReg);
prosInstruction prosBytecode_newSal_I24(prosImmediate24 srcIme);
prosInstruction prosBytecode_newSdl_R(prosRegister srcReg);
prosInstruction prosBytecode_newSdl_I24(prosImmediate24 srcIme);

void prosBytecode_dump(prosInstruction code[], size_t size);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_CODE_LITTLESENNA_BYTECODE_H
