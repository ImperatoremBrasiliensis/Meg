/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/LittleSenna/LSMachine.h>

#include <internal/Orbita.h>

#include <stdio.h>

prosLSMachine prosLSMachine_new(size_t stackSize) {
   prosLSMachine ret = {};
   ret.altor = prosAllocator_new();
   ret.stack = malloc(stackSize);
   ret.stackSize = stackSize;
   ret.sregs[PROS_SPECIAL_REGISTER_SP] = (uint64_t) ret.stack + stackSize;

   return ret;
}

void prosLSMachine_del(prosLSMachine *self) {
   prosAllocator_del(&self->altor);
   free(self->stack);
   *self = (prosLSMachine){};
}

bool prosLSMachine_exec(prosLSMachine *self, prosInstruction instrs[]) {
   self->sregs[PROS_SPECIAL_REGISTER_PC] = (uint64_t) instrs;

   while (true) {
      prosInstruction in = *(prosInstruction *) self->sregs[PROS_SPECIAL_REGISTER_PC];

      switch (in.code) {
      case PROS_INSTRUCTION_ADD_REG1_REG2_REG3:
         self->regs[in.as.add_r1_r2_r3.reg1] =
            self->regs[in.as.add_r1_r2_r3.reg2] + self->regs[in.as.add_r1_r2_r3.reg3];
         break;

      case PROS_INSTRUCTION_ADD_REG1_REG2_IME16:
         self->regs[in.as.add_r1_r2_i16.reg1] =
            self->regs[in.as.add_r1_r2_i16.reg2] + in.as.add_r1_r2_i16.ime;
         break;

      case PROS_INSTRUCTION_SUB_REG1_REG2_REG3:
         self->regs[in.as.sub_r1_r2_r3.reg1] =
            self->regs[in.as.sub_r1_r2_r3.reg2] - self->regs[in.as.sub_r1_r2_r3.reg3];
         break;

      case PROS_INSTRUCTION_SUB_REG1_REG2_IME16:
         self->regs[in.as.sub_r1_r2_i16.reg1] =
            self->regs[in.as.sub_r1_r2_i16.reg2] - in.as.sub_r1_r2_i16.ime;
         break;

      case PROS_INSTRUCTION_MUL_REG1_REG2_REG3:
         self->regs[in.as.mul_r1_r2_r3.reg1] =
            self->regs[in.as.mul_r1_r2_r3.reg2] * self->regs[in.as.mul_r1_r2_r3.reg3];
         break;

      case PROS_INSTRUCTION_MUL_REG1_REG2_IME16:
         self->regs[in.as.mul_r1_r2_i16.reg1] =
            self->regs[in.as.mul_r1_r2_i16.reg2] * in.as.mul_r1_r2_i16.ime;
         break;

      case PROS_INSTRUCTION_DIV_REG1_REG2_REG3:
         self->regs[in.as.div_r1_r2_r3.reg1] =
            self->regs[in.as.div_r1_r2_r3.reg2] / self->regs[in.as.div_r1_r2_r3.reg3];
         break;

      case PROS_INSTRUCTION_DIV_REG1_REG2_IME16:
         self->regs[in.as.div_r1_r2_i16.reg1] =
            self->regs[in.as.div_r1_r2_i16.reg2] / in.as.div_r1_r2_i16.ime;
         break;

      case PROS_INSTRUCTION_MOV_REG1_REG2:
         self->regs[in.as.mov_r1_r2.reg1] = self->regs[in.as.mov_r1_r2.reg2];
         break;

      case PROS_INSTRUCTION_MOV_REG_IME20:
         self->regs[in.as.mov_r_i20.reg] = in.as.mov_r_i20.ime;
         break;

      case PROS_INSTRUCTION_MOV_E_K_REG_IME16_LSL:
         if (in.as.movEk_r_i12_l.lsl < 4) {
            uint16_t *reg = (void *) &self->regs[in.as.movEk_r_i12_l.reg];
            reg[in.as.movEk_r_i12_l.lsl] = in.as.movEk_r_i12_l.ime;
            break;
         }
         pros_panic("prosLSMAchine_exec(): Bad LSL on `mov -k` instruction.");

      case PROS_INSTRUCTION_MOV_E_Z_REG_IME16_LSL:
         if (in.as.movEz_r_i12_l.lsl < 4) {
            self->regs[in.as.movEz_r_i12_l.reg] = 0;

            uint16_t *reg = (void *) &self->regs[in.as.movEk_r_i12_l.reg];
            reg[in.as.movEz_r_i12_l.lsl] = in.as.movEz_r_i12_l.ime;
            break;
         }
         pros_panic("prosLSMAchine_exec(): Bad LSL on `mov -z` instruction.");

      case PROS_INSTRUCTION_BRA_REG:
         self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.bra_r.reg * 4;
         continue;

      case PROS_INSTRUCTION_BRA_IME24:
         self->sregs[PROS_SPECIAL_REGISTER_PC] += (int64_t) in.as.bra_i24.ime * 4;
         continue;

      case PROS_INSTRUCTION_BRA_E_A_REG:
         pros_panic("prosLSMAchine_exec(): Attempted to execute `bra -a`.");

      case PROS_INSTRUCTION_BRL_REG:
         self->sregs[PROS_SPECIAL_REGISTER_BL] = self->sregs[PROS_SPECIAL_REGISTER_PC] + 4;
         self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.brl_r.reg * 4;
         continue;

      case PROS_INSTRUCTION_BRL_IME24:
         self->sregs[PROS_SPECIAL_REGISTER_BL] = self->sregs[PROS_SPECIAL_REGISTER_PC] + 4;
         self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.brl_i24.ime * 4;
         continue;

      case PROS_INSTRUCTION_BRL_E_A_REG:
         pros_panic("prosLSMAchine_exec(): Attempted to execute `brl -a`.");

      case PROS_INSTRUCTION_RET:
         self->sregs[PROS_SPECIAL_REGISTER_PC] = self->sregs[PROS_SPECIAL_REGISTER_BL];
         continue;

      case PROS_INSTRUCTION_EQL_IME16_REG1_REG2:
         if (self->regs[in.as.eql_i16_r1_r2.reg1] == self->regs[in.as.eql_i16_r1_r2.reg2]) {
            self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.eql_i16_r1_r2.ime * 4;
            continue;
         }
         break;

      case PROS_INSTRUCTION_EQL_IME12_REG_IME8:
         if (self->regs[in.as.eql_i16_r1_r2.reg1] == self->regs[in.as.eql_i16_r1_r2.ime]) {
            self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.eql_i12_r_i8.ime1 * 4;
            continue;
         }
         break;

      case PROS_INSTRUCTION_GTT_IME16_REG1_REG2:
         if (self->regs[in.as.eql_i16_r1_r2.reg1] > self->regs[in.as.eql_i16_r1_r2.ime]) {
            self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.gtt_i16_r1_r2.ime * 4;
            continue;
         }
         break;
      case PROS_INSTRUCTION_GTT_IME12_REG_IME8:
         if (self->regs[in.as.eql_i16_r1_r2.reg1] > self->regs[in.as.eql_i16_r1_r2.ime]) {
            self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.gtt_i12_r_i8.ime1 * 4;
            continue;
         }
         break;

      case PROS_INSTRUCTION_LTT_IME16_REG1_REG2:
         if (self->regs[in.as.eql_i16_r1_r2.reg1] < self->regs[in.as.eql_i16_r1_r2.ime]) {
            self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.ltt_i16_r1_r2.ime * 4;
            continue;
         }
         break;
      case PROS_INSTRUCTION_LTT_IME12_REG_IME8:
         if (self->regs[in.as.eql_i16_r1_r2.reg1] < self->regs[in.as.eql_i16_r1_r2.ime]) {
            self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.ltt_i12_r_i8.ime1 * 4;
            continue;
         }
         break;

      case PROS_INSTRUCTION_ISZ_IME20_REG:
         if (!self->regs[in.as.isz_ime20_r.reg]) {
            self->sregs[PROS_SPECIAL_REGISTER_PC] += in.as.isz_ime20_r.ime;
            continue;
         }
         break;

      case PROS_INSTRUCTION_SYS_IME24:
         pros_panic("prosLSMachine_exec(): Attempted to execute `sys`.");

      case PROS_INSTRUCTION_STR_SIZE_REG1_IME12_REG2: {
         void *p = (void *) ((int64_t) self->regs[in.as.str_sz_r1_i12_r2.reg1] + in.as.str_sz_r1_i12_r2.ime);

         switch (in.as.str_sz_s_i12_r.size) {
         case PROS_DWORD:
            *(uint64_t *) p = self->regs[in.as.str_sz_r1_i12_r2.reg2];
            break;
         case PROS_WORD:
            *(uint32_t *) p = self->regs[in.as.str_sz_r1_i12_r2.reg2];
            break;
         case PROS_HWORD:
            *(uint16_t *) p = self->regs[in.as.str_sz_r1_i12_r2.reg2];
            break;
         case PROS_BYTE:
            *(uint8_t *) p = self->regs[in.as.str_sz_r1_i12_r2.reg2];
            break;
         default:
            pros_panic("prosLSMachine: Received an invalid store size.");
         }
      } break;

      case PROS_INSTRUCTION_STR_SIZE_SREG_IME12_REG: {
         void *p = (void *) ((int64_t) self->sregs[in.as.str_sz_s_i12_r.sreg] + in.as.str_sz_s_i12_r.ime);

         switch (in.as.str_sz_s_i12_r.size) {
         case PROS_DWORD:
            *(uint64_t *) p = self->regs[in.as.str_sz_s_i12_r.reg];
            break;
         case PROS_WORD:
            *(uint32_t *) p = self->regs[in.as.str_sz_s_i12_r.reg];
            break;
         case PROS_HWORD:
            *(uint16_t *) p = self->regs[in.as.str_sz_s_i12_r.reg];
            break;
         case PROS_BYTE:
            *(uint8_t *) p = self->regs[in.as.str_sz_s_i12_r.reg];
            break;
         default:
            pros_panic("prosLSMachine: Received an invalid store size.");
         }
      } break;

      case PROS_INSTRUCTION_STP_REG1_IME12_REG2_REG3: {
         uint64_t *p = (void *) ((int64_t) self->regs[in.as.stp_r1_i12_r2_r3.reg1] + in.as.stp_r1_i12_r2_r3.ime);
         p[0] = self->regs[in.as.stp_r1_i12_r2_r3.reg2];
         p[1] = self->regs[in.as.stp_r1_i12_r2_r3.reg3];
      } break;

      case PROS_INSTRUCTION_STP_SREG_IME12_REG1_REG2: {
         uint64_t *p = (void *) ((int64_t) self->sregs[in.as.stp_s_i12_r1_r2.sreg] + in.as.stp_s_i12_r1_r2.ime);
         p[0] = self->regs[in.as.stp_s_i12_r1_r2.reg1];
         p[1] = self->regs[in.as.stp_s_i12_r1_r2.reg2];
      } break;

      case PROS_INSTRUCTION_LDR_SIZE_REG1_IME12_REG2: {
         void *p = (void *) ((int64_t) self->regs[in.as.ldr_sz_r1_i12_r2.reg1] + in.as.ldr_sz_r1_i12_r2.ime);

         switch (in.as.ldr_sz_s_i12_r.size) {
         case PROS_DWORD:
            self->regs[in.as.ldr_sz_r1_i12_r2.reg2] = *(uint64_t *) p;
            break;
         case PROS_WORD:
            self->regs[in.as.ldr_sz_r1_i12_r2.reg2] = *(uint32_t *) p;
            break;
         case PROS_HWORD:
            self->regs[in.as.ldr_sz_r1_i12_r2.reg2] = *(uint16_t *) p;
            break;
         case PROS_BYTE:
            self->regs[in.as.ldr_sz_r1_i12_r2.reg2] = *(uint8_t *) p;
            break;
         default:
            pros_panic("prosLSMachine: Received an invalid load size.");
         }
      } break;

      case PROS_INSTRUCTION_LDR_SIZE_SREG_IME12_REG: {
         void *p = (void *) ((int64_t) self->sregs[in.as.ldr_sz_s_i12_r.sreg] + in.as.ldr_sz_s_i12_r.ime);

         switch (in.as.ldr_sz_s_i12_r.size) {
         case PROS_DWORD:
            self->regs[in.as.ldr_sz_s_i12_r.reg] = *(uint64_t *) p;
            break;
         case PROS_WORD:
            self->regs[in.as.ldr_sz_s_i12_r.reg] = *(uint32_t *) p;
            break;
         case PROS_HWORD:
            self->regs[in.as.ldr_sz_s_i12_r.reg] = *(uint16_t *) p;
            break;
         case PROS_BYTE:
            self->regs[in.as.ldr_sz_s_i12_r.reg] = *(uint8_t *) p;
            break;
         default:
            pros_panic("prosLSMachine: Received an invalid load size.");
         }
      } break;

      case PROS_INSTRUCTION_LDP_REG1_IME12_REG2_REG3: {
         uint64_t *p = (void *) ((int64_t) self->regs[in.as.ldp_r1_i12_r2_r3.reg1] + in.as.ldp_r1_i12_r2_r3.ime);
         self->regs[in.as.ldp_r1_i12_r2_r3.reg2] = p[0];
         self->regs[in.as.ldp_r1_i12_r2_r3.reg3] = p[1];
      } break;

      case PROS_INSTRUCTION_LDP_SREG_IME12_REG1_REG2: {
         uint64_t *p = (void *) ((int64_t) self->sregs[in.as.ldp_s_i12_r1_r2.sreg] + in.as.ldp_s_i12_r1_r2.ime);
         self->regs[in.as.ldp_s_i12_r1_r2.reg1] = p[0];
         self->regs[in.as.ldp_s_i12_r1_r2.reg2] = p[1];
      } break;

      case PROS_INSTRUCTION_SAL_REG:
         self->sregs[PROS_SPECIAL_REGISTER_SP] -= self->regs[in.as.sal_r.reg];
         break;

      case PROS_INSTRUCTION_SAL_IME24:
         self->sregs[PROS_SPECIAL_REGISTER_SP] -= in.as.sal_i24.ime;
         break;

      case PROS_INSTRUCTION_SDL_REG:
         self->sregs[PROS_SPECIAL_REGISTER_SP] += self->regs[in.as.sdl_r.reg];
         break;

      case PROS_INSTRUCTION_SDL_IME24:
         self->sregs[PROS_SPECIAL_REGISTER_SP] += in.as.sdl_i24.ime;
         break;

      case PROS_INSTRUCTION_ZERO:
         puts("General registers:");
         for (size_t i = 0; i < PROS_SIZEOF_ARRAY(self->regs); i++)
            printf("  r%zu: %zu\n", i, self->regs[i]);
         puts("Special registers:");
         printf("  pc: %zu\n", self->sregs[PROS_SPECIAL_REGISTER_PC]);
         printf("  sp: %zu\n", self->sregs[PROS_SPECIAL_REGISTER_SP]);
         printf("  bl: %zu\n", self->sregs[PROS_SPECIAL_REGISTER_BL]);
         goto finalize;

      default:
         pros_panic("prosLSMachine_exec(): Invalid instruction.");
      }

      self->sregs[PROS_SPECIAL_REGISTER_PC] += 4;
   }

finalize:
   return true;
}
