#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint32_t u32;
    int32_t i32, j32;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int fixedaddress;
    MAYUSE(u16);
    MAYUSE(u8);
    MAYUSE(j32);

    while((opcode==0x2E) || (opcode==0x66))   // ignoring CS: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        

                
        case 0x0F:
            addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;

        case 0xD1:
        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    if(opcode==0xD1) {
                        INST_NAME("ROL Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROL Ew, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEW(x1, 0);
                    CALL_(rol16, x1, x3);
                    EWBACK;
                    break;
                case 1:
                    if(opcode==0xD1) {
                        INST_NAME("ROR Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROR Ew, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEW(x1, 0);
                    CALL_(ror16, x1, x3);
                    EWBACK;
                    break;
                case 2:
                    if(opcode==0xD1) {INST_NAME("RCL Ew, 1"); } else { INST_NAME("RCL Ew, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD1) {MOV32w(x2, 1);} else {ANDSw_mask(x2, xRCX, 0, 0b00100);}
                    GETEW(x1, 0);
                    CALL_(rcl16, x1, x3);
                    EWBACK;
                    break;
                case 3:
                    if(opcode==0xD1) {INST_NAME("RCR Ew, 1");} else {INST_NAME("RCR Ew, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD1) {MOV32w(x2, 1);} else {ANDSw_mask(x2, xRCX, 0, 0b00100);}
                    GETEW(x1, 0);
                    CALL_(rcr16, x1, x3);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    if(opcode==0xD1) {
                        INST_NAME("SHL Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHL Ew, CL");
                        ANDSw_mask(x4, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    LSLw_REG(ed, ed, x4);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl16);
                    break;
                case 5:
                    if(opcode==0xD1) {
                        INST_NAME("SHR Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHR Ew, CL");
                        ANDSw_mask(x4, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    LSRw_REG(ed, ed, x4);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr16);
                    break;
                case 7:
                    if(opcode==0xD1) {
                        INST_NAME("SAR Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SAR Ew, CL");
                        ANDSw_mask(x4, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    ASRw_REG(ed, ed, x4);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar16);
                    break;
            }
            break;
            
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 2);
                    u16 = F16;
                    MOV32w(x2, u16);
                    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1, 0);
                    MVNw_REG(ed, ed);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    emit_neg16(dyn, ninst, ed, x2, x4);
                    EWBACK;
                    break;
                case 4:
                    INST_NAME("MUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_mul16);
                    GETEW(x1, 0);
                    UXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    break;
                case 5:
                    INST_NAME("IMUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_imul16);
                    GETSEW(x1, 0);
                    SXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    CALL(div16, -1);
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    CALL(idiv16, -1);
                    break;
            }
            break;
            
        default:
            DEFAULT;
    }
    return addr;
}

