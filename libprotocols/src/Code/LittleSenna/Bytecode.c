/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/LittleSenna/Bytecode.h>

#include <internal/Orbita.h>

#include <stdio.h>

/* Mathematical Instructions */

prosInstruction prosBytecode_newAdd_R1_R2_R3(prosRegister destReg, prosRegister lshReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_ADD_REG1_REG2_REG3,
      .as = {
         .add_r1_r2_r3 = {
            .reg1 = destReg,
            .reg2 = lshReg,
            .reg3 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newAdd_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_ADD_REG1_REG2_IME16,
      .as = {
         .add_r1_r2_i16 = {
            .reg1 = destReg,
            .reg2 = lhsReg,
            .ime = rhsIme
         }
      }
   };
}

prosInstruction prosBytecode_newSub_R1_R2_R3(prosRegister destReg, prosRegister lshReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_SUB_REG1_REG2_REG3,
      .as = {
         .sub_r1_r2_r3 = {
            .reg1 = destReg,
            .reg2 = lshReg,
            .reg3 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newSub_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_SUB_REG1_REG2_IME16,
      .as = {
         .sub_r1_r2_i16 = {
            .reg1 = destReg,
            .reg2 = lhsReg,
            .ime = rhsIme
         }
      }
   };
}

prosInstruction prosBytecode_newMul_R1_R2_R3(prosRegister destReg, prosRegister lshReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_MUL_REG1_REG2_REG3,
      .as = {
         .mul_r1_r2_r3 = {
            .reg1 = destReg,
            .reg2 = lshReg,
            .reg3 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newMul_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_MUL_REG1_REG2_IME16,
      .as = {
         .mul_r1_r2_i16 = {
            .reg1 = destReg,
            .reg2 = lhsReg,
            .ime = rhsIme
         }
      }
   };
}

prosInstruction prosBytecode_newDiv_R1_R2_R3(prosRegister destReg, prosRegister lshReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_DIV_REG1_REG2_REG3,
      .as = {
         .div_r1_r2_r3 = {
            .reg1 = destReg,
            .reg2 = lshReg,
            .reg3 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newDiv_R1_R2_I16(prosRegister destReg, prosRegister lhsReg, prosImmediate16 rhsIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_DIV_REG1_REG2_IME16,
      .as = {
         .div_r1_r2_i16 = {
            .reg1 = destReg,
            .reg2 = lhsReg,
            .ime = rhsIme
         }
      }
   };
}

/* Movement Instructions */

prosInstruction prosBytecode_newMov_R1_R2(prosRegister destReg, prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_MOV_REG1_REG2,
      .as = {
         .mov_r1_r2 = {
            .reg1 = destReg,
            .reg2 = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newMov_R_I20(prosRegister destReg, prosImmediate20 srcIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_MOV_REG_IME20,
      .as = {
         .mov_r_i20 = {
            .reg = destReg,
            .ime = srcIme
         }
      }
   };
}

prosInstruction prosBytecode_newMovEk_R_I16_L(prosRegister destReg, prosImmediate16 srcIme, prosLogicalShiftLeft lsl) {
   if (lsl >= 4) {
      pros_panic(
         "prosBytecode_newMovEk_R_I16_L():"
         " Logical Shift Left must be `PROS_LSL_00`, `PROS_LSL_16`, `PROS_LSL_32` or `PROS_LSL_48`."
      );
   }

   return (prosInstruction){
      .code = PROS_INSTRUCTION_MOV_E_K_REG_IME16_LSL,
      .as = {
         .movEk_r_i12_l = {
            .reg = destReg,
            .ime = srcIme,
            .lsl = lsl
         }
      }
   };
}

prosInstruction prosBytecode_newMovEz_R_I16_L(prosRegister destReg, prosImmediate16 srcIme, prosLogicalShiftLeft lsl) {
   if (lsl >= 4) {
      pros_panic(
         "prosBytecode_newMovEz_R_I16_L():"
         " Logical Shift Left must be `PROS_LSL_00`, `PROS_LSL_16`, `PROS_LSL_32` or `PROS_LSL_48`."
      );
   }

   return (prosInstruction){
      .code = PROS_INSTRUCTION_MOV_E_Z_REG_IME16_LSL,
      .as = {
         .movEz_r_i12_l = {
            .reg = destReg,
            .ime = srcIme,
            .lsl = lsl
         }
      }
   };
}

prosInstruction prosBytecode_newAnd_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_AND_REG1_REG2_REG3,
      .as = {
         .and_r1_r2_r3 = {
            .reg1 = destReg,
            .reg2 = lhsReg,
            .reg3 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newLor_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_LOR_REG1_REG2_REG3,
      .as = {
         .lor_r1_r2_r3 = {
            .reg1 = destReg,
            .reg2 = lhsReg,
            .reg3 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newEor_R1_R2_R3(prosRegister destReg, prosRegister lhsReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_EOR_REG1_REG2_REG3,
      .as = {
         .eor_r1_r2_r3 = {
            .reg1 = destReg,
            .reg2 = lhsReg,
            .reg3 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newNot_R1_R2(prosRegister destReg, prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_NOT_REG1_REG2,
      .as = {
         .not_r1_r2 = {
            .reg1 = destReg,
            .reg2 = srcReg
         }
      }
   };
}

/* Logical Instructions */

prosInstruction prosBytecode_newBra_R(prosRegister offsetReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_BRA_REG,
      .as = {
         .bra_r = {
            .reg = offsetReg
         }
      }
   };
}

prosInstruction prosBytecode_newBra_I24(prosImmediate24 offsetIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_BRA_IME24,
      .as = {
         .bra_i24 = {
            .ime = offsetIme
         }
      }
   };
}

prosInstruction prosBytecode_newBraEa_R(prosRegister adrReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_BRA_E_A_REG,
      .as = {
         .braEa_r = {
            .reg = adrReg
         }
      }
   };
}

prosInstruction prosBytecode_newBrl_R(prosRegister offsetReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_BRL_REG,
      .as = {
         .brl_r = {
            .reg = offsetReg
         }
      }
   };
}

prosInstruction prosBytecode_newBrl_I24(prosImmediate24 offsetIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_BRL_IME24,
      .as = {
         .brl_i24 = {
            .ime = offsetIme
         }
      }
   };
}

prosInstruction prosBytecode_newBrlEa_R(prosRegister adrReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_BRL_E_A_REG,
      .as = {
         .brlEa_r = {
            .reg = adrReg
         }
      }
   };
}

prosInstruction prosBytecode_newRet() {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_RET
   };
}

/* Comparison Instructions */

prosInstruction prosBytecode_newEql_I16_R1_R2(prosImmediate16 offsetIme, prosRegister lhsReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_EQL_IME16_REG1_REG2,
      .as = {
         .eql_i16_r1_r2 = {
            .ime = offsetIme,
            .reg1 = lhsReg,
            .reg2 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newEql_I12_R_I8(prosImmediate12 offsetIme, prosRegister lhsReg, prosImmediate8 rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_EQL_IME12_REG_IME8,
      .as = {
         .eql_i12_r_i8 = {
            .ime1 = offsetIme,
            .reg = lhsReg,
            .ime2 = rhsReg
         }
      }
   };
}
prosInstruction prosBytecode_newGtt_I16_R1_R2(prosImmediate16 offsetIme, prosRegister lhsReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_GTT_IME16_REG1_REG2,
      .as = {
         .gtt_i16_r1_r2 = {
            .ime = offsetIme,
            .reg1 = lhsReg,
            .reg2 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newGtt_I12_R_I8(prosImmediate12 offsetIme, prosRegister lhsReg, prosImmediate8 rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_GTT_IME12_REG_IME8,
      .as = {
         .gtt_i12_r_i8 = {
            .ime1 = offsetIme,
            .reg = lhsReg,
            .ime2 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newLtt_I16_R1_R2(prosImmediate16 offsetIme, prosRegister lhsReg, prosRegister rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_LTT_IME16_REG1_REG2,
      .as = {
         .ltt_i16_r1_r2 = {
            .ime = offsetIme,
            .reg1 = lhsReg,
            .reg2 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newLtt_I12_R_I8(prosImmediate12 offsetIme, prosRegister lhsReg, prosImmediate8 rhsReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_LTT_IME12_REG_IME8,
      .as = {
         .ltt_i12_r_i8 = {
            .ime1 = offsetIme,
            .reg = lhsReg,
            .ime2 = rhsReg
         }
      }
   };
}

prosInstruction prosBytecode_newIsz_I20_R(prosImmediate20 offsetIme, prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_ISZ_IME20_REG,
      .as = {
         .isz_ime20_r = {
            .ime = offsetIme,
            .reg = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newSys_I24(prosImmediate24 codeIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_SYS_IME24,
      .as = {
         .sys_i24 = {
            .ime = codeIme
         }
      }
   };
}

prosInstruction prosBytecode_newStr_SZ_R1_I12_R2(prosWordSize size, prosRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_STR_SIZE_REG1_IME12_REG2,
      .as = {
         .str_sz_r1_i12_r2 = {
            .size = size,
            .reg1 = addrReg,
            .ime = offsetIme,
            .reg2 = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newStr_SZ_S_I12_R(prosWordSize size, prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_STR_SIZE_SREG_IME12_REG,
      .as = {
         .str_sz_s_i12_r = {
            .size = size,
            .sreg = addrReg,
            .ime = offsetIme,
            .reg = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newStp_R1_I12_R2_R3(prosRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg1, prosRegister srcReg2) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_STP_REG1_IME12_REG2_REG3,
      .as = {
         .stp_r1_i12_r2_r3 = {
            .reg1 = addrReg,
            .ime = offsetIme,
            .reg2 = srcReg1,
            .reg3 = srcReg2
         }
      }
   };
}

prosInstruction prosBytecode_newStp_S_I12_R1_R2(prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg1, prosRegister srcReg2) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_STP_SREG_IME12_REG1_REG2,
      .as = {
         .stp_s_i12_r1_r2 = {
            .sreg = addrReg,
            .ime = offsetIme,
            .reg1 = srcReg1,
            .reg2 = srcReg2
         }
      }
   };
}

prosInstruction prosBytecode_newLdr_SZ_R1_I12_R2(prosWordSize size, prosRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_LDR_SIZE_REG1_IME12_REG2,
      .as = {
         .ldr_sz_r1_i12_r2 = {
            .size = size,
            .reg1 = addrReg,
            .ime = offsetIme,
            .reg2 = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newLdr_SZ_S_I12_R(prosWordSize size, prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_LDR_SIZE_SREG_IME12_REG,
      .as = {
         .ldr_sz_s_i12_r = {
            .size = size,
            .sreg = addrReg,
            .ime = offsetIme,
            .reg = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newLdp_R1_I12_R2_R3(prosRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg1, prosRegister srcReg2) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_LDP_REG1_IME12_REG2_REG3,
      .as = {
         .ldp_r1_i12_r2_r3 = {
            .reg1 = addrReg,
            .ime = offsetIme,
            .reg2 = srcReg1,
            .reg3 = srcReg2
         }
      }
   };
}

prosInstruction prosBytecode_newLdp_S_I12_R1_R2(prosSpecialRegister addrReg, prosImmediate12 offsetIme, prosRegister srcReg1, prosRegister srcReg2) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_LDP_SREG_IME12_REG1_REG2,
      .as = {
         .ldp_s_i12_r1_r2 = {
            .sreg = addrReg,
            .ime = offsetIme,
            .reg1 = srcReg1,
            .reg2 = srcReg2
         }
      }
   };
}

prosInstruction prosBytecode_newSal_R(prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_SAL_REG,
      .as = {
         .sal_r = {
            .reg = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newSal_I24(prosImmediate24 srcIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_SAL_IME24,
      .as = {
         .sal_i24 = {
            .ime = srcIme
         }
      }
   };
}

prosInstruction prosBytecode_newSdl_R(prosRegister srcReg) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_SDL_REG,
      .as = {
         .sdl_r = {
            .reg = srcReg
         }
      }
   };
}

prosInstruction prosBytecode_newSdl_I24(prosImmediate24 srcIme) {
   return (prosInstruction){
      .code = PROS_INSTRUCTION_SDL_IME24,
      .as = {
         .sdl_i24 = {
            .ime = srcIme
         }
      }
   };
}

void prosBytecode_dump(prosInstruction code[], size_t size) {
   size_t i = 0;
   while (i < size) {
      prosInstruction in = code[i];

      switch (in.code) {
      case PROS_INSTRUCTION_ZERO:
         puts("{}");
         break;

      case PROS_INSTRUCTION_ADD_REG1_REG2_REG3:
         auto add1 = in.as.add_r1_r2_r3;
         printf("add r%u, r%u, r%u\n", add1.reg1, add1.reg2, add1.reg3);
         break;

      case PROS_INSTRUCTION_ADD_REG1_REG2_IME16:
         auto add2 = in.as.add_r1_r2_i16;
         printf("add r%u, r%u, #%i\n", add2.reg1, add2.reg2, (int16_t) add2.ime);
         break;

      case PROS_INSTRUCTION_SUB_REG1_REG2_REG3:
         auto sub1 = in.as.sub_r1_r2_r3;
         printf("sub r%u, r%u, r%u\n", sub1.reg1, sub1.reg2, sub1.reg3);
         break;

      case PROS_INSTRUCTION_SUB_REG1_REG2_IME16:
         auto sub2 = in.as.sub_r1_r2_i16;
         printf("sub r%u, r%u, #%i\n", sub2.reg1, sub2.reg2, (int16_t) sub2.ime);
         break;

      case PROS_INSTRUCTION_MUL_REG1_REG2_REG3:
         auto mul1 = in.as.mul_r1_r2_r3;
         printf("mul r%u, r%u, r%u\n", mul1.reg1, mul1.reg2, mul1.reg3);
         break;

      case PROS_INSTRUCTION_MUL_REG1_REG2_IME16:
         auto mul2 = in.as.mul_r1_r2_i16;
         printf("mul r%u, r%u, #%i\n", mul2.reg1, mul2.reg2, (int16_t) mul2.ime);
         break;

      case PROS_INSTRUCTION_DIV_REG1_REG2_REG3:
         auto div1 = in.as.div_r1_r2_r3;
         printf("div r%u, r%u, r%u\n", div1.reg1, div1.reg2, div1.reg3);
         break;

      case PROS_INSTRUCTION_DIV_REG1_REG2_IME16:
         auto div2 = in.as.div_r1_r2_i16;
         printf("div r%u, r%u, #%i\n", div2.reg1, div2.reg2, (int16_t) div2.ime);
         break;

      case PROS_INSTRUCTION_MOV_REG1_REG2:
         auto mov1 = in.as.mov_r1_r2;
         printf("mov r%u, r%u\n", mov1.reg1, mov1.reg2);
         break;

      case PROS_INSTRUCTION_MOV_REG_IME20:
         auto mov2 = in.as.mov_r_i20;
         printf("mov r%u, #%i\n", mov2.reg, (int32_t) mov2.ime);
         break;

      case PROS_INSTRUCTION_MOV_E_K_REG_IME16_LSL:
         auto mov_k = in.as.movEk_r_i12_l;
         printf("mov -k r%u, #%i, #%i\n", mov_k.reg, (int16_t) mov_k.ime, (uint8_t) mov_k.lsl);
         break;

      case PROS_INSTRUCTION_MOV_E_Z_REG_IME16_LSL:
         auto mov_z = in.as.movEk_r_i12_l;
         printf("mov -z r%u, #%i, #%i\n", mov_z.reg, (int16_t) mov_z.ime, (uint8_t) mov_z.lsl);
         break;

      case PROS_INSTRUCTION_AND_REG1_REG2_REG3:
         auto and = in.as.and_r1_r2_r3;
         printf("and r%u, r%u, r%u\n", and.reg1, and.reg2, and.reg3);
         break;

      case PROS_INSTRUCTION_LOR_REG1_REG2_REG3:
         auto lor = in.as.and_r1_r2_r3;
         printf("lor r%u, r%u, r%u\n", lor.reg1, lor.reg2, lor.reg3);
         break;

      case PROS_INSTRUCTION_EOR_REG1_REG2_REG3:
         auto eor = in.as.eor_r1_r2_r3;
         printf("eor r%u, r%u, r%u\n", eor.reg1, eor.reg2, eor.reg3);
         break;

      case PROS_INSTRUCTION_NOT_REG1_REG2:
         auto not = in.as.not_r1_r2;
         printf("not r%u, r%u\n", not.reg1, not.reg2);
         break;

      case PROS_INSTRUCTION_BRA_REG:
         auto bra1 = in.as.bra_r;
         printf("bra r%u\n", bra1.reg);
         break;

      case PROS_INSTRUCTION_BRA_IME24:
         auto bra2 = in.as.bra_i24;
         printf("bra #%i\n", (int32_t) bra2.ime);
         break;

      case PROS_INSTRUCTION_BRA_E_A_REG:
         auto bra_a = in.as.braEa_r;
         printf("bra -a r%u\n", bra_a.reg);
         break;

      case PROS_INSTRUCTION_BRL_REG:
         auto brl1 = in.as.brl_r;
         printf("brl r%u\n", brl1.reg);
         break;

      case PROS_INSTRUCTION_BRL_IME24:
         auto brl2 = in.as.brl_i24;
         printf("brl #%i\n", (int32_t) brl2.ime);
         break;

      case PROS_INSTRUCTION_BRL_E_A_REG:
         auto brl_a = in.as.brlEa_r;
         printf("brl -a r%u\n", brl_a.reg);
         break;

      case PROS_INSTRUCTION_RET:
         puts("ret");
         break;

      case PROS_INSTRUCTION_STR_SIZE_REG1_IME12_REG2:
         auto str1 = in.as.str_sz_r1_i12_r2;
         printf("str #%u, r%u, #%i, r%u\n", (uint8_t) str1.size, str1.reg1, (int16_t) str1.ime, str1.reg2);
         break;

      case PROS_INSTRUCTION_STR_SIZE_SREG_IME12_REG:
         auto str2 = in.as.str_sz_s_i12_r;
         printf("str #%u, s%u, #%i, r%u\n", (uint8_t) str2.size, str2.sreg, (int16_t) str2.ime, str2.reg);
         break;

      case PROS_INSTRUCTION_LDR_SIZE_REG1_IME12_REG2:
         auto ldr1 = in.as.ldr_sz_r1_i12_r2;
         printf("ldr #%u, r%u, #%i, r%u\n", (uint8_t) ldr1.size, ldr1.reg1, (int16_t) ldr1.ime, ldr1.reg2);
         break;

      case PROS_INSTRUCTION_LDR_SIZE_SREG_IME12_REG:
         auto ldr2 = in.as.ldr_sz_s_i12_r;
         printf("ldr #%u, s%u, #%i, r%u\n", (uint8_t) ldr2.size, ldr2.sreg, (int16_t) ldr2.ime, ldr2.reg);
         break;

      case PROS_INSTRUCTION_SAL_REG:
         auto sal1 = in.as.sal_r;
         printf("sal r%u\n", sal1.reg);
         break;

      case PROS_INSTRUCTION_SAL_IME24:
         auto sal2 = in.as.sal_i24;
         printf("sal #%i\n", (int32_t) sal2.ime);
         break;

      case PROS_INSTRUCTION_SDL_REG:
         auto sdl1 = in.as.sdl_r;
         printf("sdl r%u\n", sdl1.reg);
         break;

      case PROS_INSTRUCTION_SDL_IME24:
         auto sdl2 = in.as.sdl_i24;
         printf("sdl #%i\n", (int32_t) sdl2.ime);
         break;

      default:
         puts("{} unknown");
      }

      i++;
   }
}
