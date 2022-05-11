#ifndef REGISTER_MAPPINGS_H
#define REGISTER_MAPPINGS_H

#include <map>
#include "csr_list.h"

static const std::map<std::string, uint32_t> register_name_map = {
  { "RZero", 0x0 },
  { "RA", 0x1 },
  { "SP", 0x2 },
  { "GP", 0x3 },
  { "TP", 0x4 },
  { "T0", 0x5 },
  { "T1", 0x6 },
  { "T2", 0x7 },
  { "S0", 0x8 },
  { "S1", 0x9 },
  { "A0", 0xa },
  { "A1", 0xb },
  { "A2", 0xc },
  { "A3", 0xd },
  { "A4", 0xe },
  { "A5", 0xf },
  { "A6", 0x10 },
  { "A7", 0x11 },
  { "S2", 0x12 },
  { "S3", 0x13 },
  { "S4", 0x14 },
  { "S5", 0x15 },
  { "S6", 0x16 },
  { "S7", 0x17 },
  { "S8", 0x18 },
  { "S9", 0x19 },
  { "S10", 0x1a },
  { "S11", 0x1b },
  { "T3", 0x1c },
  { "T4", 0x1d },
  { "T5", 0x1e },
  { "T6", 0x1f },
  { "FT0", 0x20 },
  { "FT1", 0x21 },
  { "FT2", 0x22 },
  { "FT3", 0x23 },
  { "FT4", 0x24 },
  { "FT5", 0x25 },
  { "FT6", 0x26 },
  { "FT7", 0x27 },
  { "FS0", 0x28 },
  { "FS1", 0x29 },
  { "FA0", 0x2a },
  { "FA1", 0x2b },
  { "FA2", 0x2c },
  { "FA3", 0x2d },
  { "FA4", 0x2e },
  { "FA5", 0x2f },
  { "FA6", 0x30 },
  { "FA7", 0x31 },
  { "FS2", 0x32 },
  { "FS3", 0x33 },
  { "FS4", 0x34 },
  { "FS5", 0x35 },
  { "FS6", 0x36 },
  { "FS7", 0x37 },
  { "FS8", 0x38 },
  { "FS9", 0x39 },
  { "FS10", 0x3a },
  { "FS11", 0x3b },
  { "FT8", 0x3c },
  { "FT9", 0x3d },
  { "FT10", 0x3e },
  { "FT11", 0x3f },
  { "PC", 0x40 }
};

// DPL CSR names to hardware tag address
static const std::map<std::string, uint32_t> csr_name_map = {
  { "TSelect", 0x0 },
  { "TData1", 0x0 },
  { "TData2", 0x0 },
  { "TData3", 0x0 },
  { "DCSR", 0x0 },
  { "DPC", 0x0 },
  { "DScratch", 0x1 },
  { "MVendorID", 0x2 },
  { "MArchID", 0x2 },
  { "MIMPID", 0x2 },
  { "MHartID", 0x2 },
  { "MStatus", 0x2 },
  { "MISA", 0x2 },
  { "MEDeleg", 0x2 },
  { "MIDeleg", 0x2 },
  { "MIE", 0x2 },
  { "MTVec", 0x2 },
  { "MCounterEN", 0x2 },
  { "MCause", 0x2 },
  { "MIP", 0x2 },
  { "MScratch", 0x3 },
  { "MEPC", 0x4 },
  { "MTVal", 0x5 },
  { "PMPCFG0", 0x6 },
  { "PMPCFG1", 0x6 },
  { "PMPCFG2", 0x6 },
  { "PMPCFG3", 0x6 },
  { "PMPAddr0", 0x6 },
  { "PMPAddr1", 0x6 },
  { "PMPAddr2", 0x6 },
  { "PMPAddr3", 0x6 },
  { "PMPAddr4", 0x6 },
  { "PMPAddr5", 0x6 },
  { "PMPAddr6", 0x6 },
  { "PMPAddr7", 0x6 },
  { "PMPAddr8", 0x6 },
  { "PMPAddr9", 0x6 },
  { "PMPAddr10", 0x6 },
  { "PMPAddr11", 0x6 },
  { "PMPAddr12", 0x6 },
  { "PMPAddr13", 0x6 },
  { "PMPAddr14", 0x6 },
  { "PMPAddr15", 0x6 },
  { "MCycle", 0x7 },
  { "MInstRet", 0x7 },
  { "MHPMCounter3", 0x7 },
  { "MHPMCounter4", 0x7 },
  { "MHPMCounter5", 0x7 },
  { "MHPMCounter6", 0x7 },
  { "MHPMCounter7", 0x7 },
  { "MHPMCounter8", 0x7 },
  { "MHPMCounter9", 0x7 },
  { "MHPMCounter10", 0x7 },
  { "MHPMCounter11", 0x7 },
  { "MHPMCounter12", 0x7 },
  { "MHPMCounter13", 0x7 },
  { "MHPMCounter14", 0x7 },
  { "MHPMCounter15", 0x7 },
  { "MHPMCounter16", 0x7 },
  { "MHPMCounter17", 0x7 },
  { "MHPMCounter18", 0x7 },
  { "MHPMCounter19", 0x7 },
  { "MHPMCounter20", 0x7 },
  { "MHPMCounter21", 0x7 },
  { "MHPMCounter22", 0x7 },
  { "MHPMCounter23", 0x7 },
  { "MHPMCounter24", 0x7 },
  { "MHPMCounter25", 0x7 },
  { "MHPMCounter26", 0x7 },
  { "MHPMCounter27", 0x7 },
  { "MHPMCounter28", 0x7 },
  { "MHPMCounter29", 0x7 },
  { "MHPMCounter30", 0x7 },
  { "MHPMCounter31", 0x7 },
  { "MHPMEvent3", 0x7 },
  { "MHPMEvent4", 0x7 },
  { "MHPMEvent5", 0x7 },
  { "MHPMEvent6", 0x7 },
  { "MHPMEvent7", 0x7 },
  { "MHPMEvent8", 0x7 },
  { "MHPMEvent9", 0x7 },
  { "MHPMEvent10", 0x7 },
  { "MHPMEvent11", 0x7 },
  { "MHPMEvent12", 0x7 },
  { "MHPMEvent13", 0x7 },
  { "MHPMEvent14", 0x7 },
  { "MHPMEvent15", 0x7 },
  { "MHPMEvent16", 0x7 },
  { "MHPMEvent17", 0x7 },
  { "MHPMEvent18", 0x7 },
  { "MHPMEvent19", 0x7 },
  { "MHPMEvent20", 0x7 },
  { "MHPMEvent21", 0x7 },
  { "MHPMEvent22", 0x7 },
  { "MHPMEvent23", 0x7 },
  { "MHPMEvent24", 0x7 },
  { "MHPMEvent25", 0x7 },
  { "MHPMEvent26", 0x7 },
  { "MHPMEvent27", 0x7 },
  { "MHPMEvent28", 0x7 },
  { "MHPMEvent29", 0x7 },
  { "MHPMEvent30", 0x7 },
  { "MHPMEvent31", 0x7 },
  { "UStatus", 0x8 },
  { "UIE", 0x8 },
  { "UTVec", 0x8 },
  { "UCause", 0x8 },
  { "UIP", 0x8 },
  { "UScratch", 0x9 },
  { "UEPC", 0xa },
  { "UTVal", 0xb },
  { "FFlags", 0xc },
  { "Cycle", 0xd },
  { "Time", 0xd },
  { "InstRet", 0xd },
  { "HPMCounter3", 0xd },
  { "HPMCounter4", 0xd },
  { "HPMCounter5", 0xd },
  { "HPMCounter6", 0xd },
  { "HPMCounter7", 0xd },
  { "HPMCounter8", 0xd },
  { "HPMCounter9", 0xd },
  { "HPMCounter10", 0xd },
  { "HPMCounter11", 0xd },
  { "HPMCounter12", 0xd },
  { "HPMCounter13", 0xd },
  { "HPMCounter14", 0xd },
  { "HPMCounter15", 0xd },
  { "HPMCounter16", 0xd },
  { "HPMCounter17", 0xd },
  { "HPMCounter18", 0xd },
  { "HPMCounter19", 0xd },
  { "HPMCounter20", 0xd },
  { "HPMCounter21", 0xd },
  { "HPMCounter22", 0xd },
  { "HPMCounter23", 0xd },
  { "HPMCounter24", 0xd },
  { "HPMCounter25", 0xd },
  { "HPMCounter26", 0xd },
  { "HPMCounter27", 0xd },
  { "HPMCounter28", 0xd },
  { "HPMCounter29", 0xd },
  { "HPMCounter30", 0xd },
  { "HPMCounter31", 0xd },
  { "SStatus", 0xe }, // missing sedeleg and sideleg
  { "SIE", 0xe },
  { "STVec", 0xe },
  { "SCounterEN", 0xe },
  { "SCause", 0xe },
  { "SIP", 0xe },
  { "SATP", 0xe },
  { "SScratch", 0xf },
  { "SEPC", 0x10 },
  { "STVal", 0x11 },
};

// DPL names for CSRs mapped to host address
static const std::map<std::string, uint32_t> csr_host_name_map = {
  { "FFlags", CSR_FFLAGS },
  { "FRM", CSR_FRM },
  { "FCSR", CSR_FCSR },
  { "UStatus", CSR_USTATUS },
  { "UIE", CSR_UIE },
  { "UTVec", CSR_UTVEC },
  { "VSTART", CSR_VSTART },
  { "VXSAT", CSR_VXSAT },
  { "VXRM", CSR_VXRM },
  { "UScratch", CSR_USCRATCH },
  { "UEPC", CSR_UEPC },
  { "UCause", CSR_UCAUSE },
  { "UTVal", CSR_UTVAL },
  { "UIP", CSR_UIP },
  { "Cycle", CSR_CYCLE },
  { "Time", CSR_TIME },
  { "InstRet", CSR_INSTRET },
  { "HPMCounter3", CSR_HPMCOUNTER3 },
  { "HPMCounter4", CSR_HPMCOUNTER4 },
  { "HPMCounter5", CSR_HPMCOUNTER5 },
  { "HPMCounter6", CSR_HPMCOUNTER6 },
  { "HPMCounter7", CSR_HPMCOUNTER7 },
  { "HPMCounter8", CSR_HPMCOUNTER8 },
  { "HPMCounter9", CSR_HPMCOUNTER9 },
  { "HPMCounter10", CSR_HPMCOUNTER10 },
  { "HPMCounter11", CSR_HPMCOUNTER11 },
  { "HPMCounter12", CSR_HPMCOUNTER12 },
  { "HPMCounter13", CSR_HPMCOUNTER13 },
  { "HPMCounter14", CSR_HPMCOUNTER14 },
  { "HPMCounter15", CSR_HPMCOUNTER15 },
  { "HPMCounter16", CSR_HPMCOUNTER16 },
  { "HPMCounter17", CSR_HPMCOUNTER17 },
  { "HPMCounter18", CSR_HPMCOUNTER18 },
  { "HPMCounter19", CSR_HPMCOUNTER19 },
  { "HPMCounter20", CSR_HPMCOUNTER20 },
  { "HPMCounter21", CSR_HPMCOUNTER21 },
  { "HPMCounter22", CSR_HPMCOUNTER22 },
  { "HPMCounter23", CSR_HPMCOUNTER23 },
  { "HPMCounter24", CSR_HPMCOUNTER24 },
  { "HPMCounter25", CSR_HPMCOUNTER25 },
  { "HPMCounter26", CSR_HPMCOUNTER26 },
  { "HPMCounter27", CSR_HPMCOUNTER27 },
  { "HPMCounter28", CSR_HPMCOUNTER28 },
  { "HPMCounter29", CSR_HPMCOUNTER29 },
  { "HPMCounter30", CSR_HPMCOUNTER30 },
  { "HPMCounter31", CSR_HPMCOUNTER31 },
  { "VL", CSR_VL },
  { "VType", CSR_VTYPE },
  { "SStatus", CSR_SSTATUS },
  { "SIE", CSR_SIE },
  { "STVec", CSR_STVEC },
  { "SCounterEN", CSR_SCOUNTEREN },
  { "SScratch", CSR_SSCRATCH },
  { "SEPC", CSR_SEPC },
  { "SCause", CSR_SCAUSE },
  { "STVal", CSR_STVAL },
  { "SIP", CSR_SIP },
  { "SATP", CSR_SATP },
  { "VSStatus", CSR_VSSTATUS },
  { "VSIE", CSR_VSIE },
  { "VSTVec", CSR_VSTVEC },
  { "VSScratch", CSR_VSSCRATCH },
  { "VSEPC", CSR_VSEPC },
  { "VSCause", CSR_VSCAUSE },
  { "VSTVal", CSR_VSTVAL },
  { "VSIP", CSR_VSIP },
  { "VSATP", CSR_VSATP },
  { "HStatus", CSR_HSTATUS },
  { "HEDeleg", CSR_HEDELEG },
  { "HIDeleg", CSR_HIDELEG },
  { "HCounterEN", CSR_HCOUNTEREN },
  { "HGATP", CSR_HGATP },
  { "UTVT", CSR_UTVT },
  { "UNXTI", CSR_UNXTI },
  { "UINTStatus", CSR_UINTSTATUS },
  { "UScratchCSW", CSR_USCRATCHCSW },
  { "UScratchCSWL", CSR_USCRATCHCSWL },
  { "STVT", CSR_STVT },
  { "SNXTI", CSR_SNXTI },
  { "SINTStatus", CSR_SINTSTATUS },
  { "SScratchCSW", CSR_SSCRATCHCSW },
  { "SScratchCSWL", CSR_SSCRATCHCSWL },
  { "MTVT", CSR_MTVT },
  { "MNXTI", CSR_MNXTI },
  { "MINTStatus", CSR_MINTSTATUS },
  { "MScratchCSW", CSR_MSCRATCHCSW },
  { "MScratchCSWL", CSR_MSCRATCHCSWL },
  { "MStatus", CSR_MSTATUS },
  { "MISA", CSR_MISA },
  { "MEDeleg", CSR_MEDELEG },
  { "MIDeleg", CSR_MIDELEG },
  { "MIE", CSR_MIE },
  { "MTVec", CSR_MTVEC },
  { "MCounterEN", CSR_MCOUNTEREN },
  { "MScratch", CSR_MSCRATCH },
  { "MEPC", CSR_MEPC },
  { "MCause", CSR_MCAUSE },
  { "MTVal", CSR_MTVAL },
  { "MIP", CSR_MIP },
  { "PMPCFG0", CSR_PMPCFG0 },
  { "PMPCFG1", CSR_PMPCFG1 },
  { "PMPCFG2", CSR_PMPCFG2 },
  { "PMPCFG3", CSR_PMPCFG3 },
  { "PMPAddr0", CSR_PMPADDR0 },
  { "PMPAddr1", CSR_PMPADDR1 },
  { "PMPAddr2", CSR_PMPADDR2 },
  { "PMPAddr3", CSR_PMPADDR3 },
  { "PMPAddr4", CSR_PMPADDR4 },
  { "PMPAddr5", CSR_PMPADDR5 },
  { "PMPAddr6", CSR_PMPADDR6 },
  { "PMPAddr7", CSR_PMPADDR7 },
  { "PMPAddr8", CSR_PMPADDR8 },
  { "PMPAddr9", CSR_PMPADDR9 },
  { "PMPAddr10", CSR_PMPADDR10 },
  { "PMPAddr11", CSR_PMPADDR11 },
  { "PMPAddr12", CSR_PMPADDR12 },
  { "PMPAddr13", CSR_PMPADDR13 },
  { "PMPAddr14", CSR_PMPADDR14 },
  { "PMPAddr15", CSR_PMPADDR15 },
  { "TSelect", CSR_TSELECT },
  { "TData1", CSR_TDATA1 },
  { "TData2", CSR_TDATA2 },
  { "TData3", CSR_TDATA3 },
  { "DCSR", CSR_DCSR },
  { "DPC", CSR_DPC },
  { "DScratch", CSR_DSCRATCH },
  { "MCycle", CSR_MCYCLE },
  { "MInstRet", CSR_MINSTRET },
  { "MHPMCounter3", CSR_MHPMCOUNTER3 },
  { "MHPMCounter4", CSR_MHPMCOUNTER4 },
  { "MHPMCounter5", CSR_MHPMCOUNTER5 },
  { "MHPMCounter6", CSR_MHPMCOUNTER6 },
  { "MHPMCounter7", CSR_MHPMCOUNTER7 },
  { "MHPMCounter8", CSR_MHPMCOUNTER8 },
  { "MHPMCounter9", CSR_MHPMCOUNTER9 },
  { "MHPMCounter10", CSR_MHPMCOUNTER10 },
  { "MHPMCounter11", CSR_MHPMCOUNTER11 },
  { "MHPMCounter12", CSR_MHPMCOUNTER12 },
  { "MHPMCounter13", CSR_MHPMCOUNTER13 },
  { "MHPMCounter14", CSR_MHPMCOUNTER14 },
  { "MHPMCounter15", CSR_MHPMCOUNTER15 },
  { "MHPMCounter16", CSR_MHPMCOUNTER16 },
  { "MHPMCounter17", CSR_MHPMCOUNTER17 },
  { "MHPMCounter18", CSR_MHPMCOUNTER18 },
  { "MHPMCounter19", CSR_MHPMCOUNTER19 },
  { "MHPMCounter20", CSR_MHPMCOUNTER20 },
  { "MHPMCounter21", CSR_MHPMCOUNTER21 },
  { "MHPMCounter22", CSR_MHPMCOUNTER22 },
  { "MHPMCounter23", CSR_MHPMCOUNTER23 },
  { "MHPMCounter24", CSR_MHPMCOUNTER24 },
  { "MHPMCounter25", CSR_MHPMCOUNTER25 },
  { "MHPMCounter26", CSR_MHPMCOUNTER26 },
  { "MHPMCounter27", CSR_MHPMCOUNTER27 },
  { "MHPMCounter28", CSR_MHPMCOUNTER28 },
  { "MHPMCounter29", CSR_MHPMCOUNTER29 },
  { "MHPMCounter30", CSR_MHPMCOUNTER30 },
  { "MHPMCounter31", CSR_MHPMCOUNTER31 },
  { "MHPMEvent3", CSR_MHPMEVENT3 },
  { "MHPMEvent4", CSR_MHPMEVENT4 },
  { "MHPMEvent5", CSR_MHPMEVENT5 },
  { "MHPMEvent6", CSR_MHPMEVENT6 },
  { "MHPMEvent7", CSR_MHPMEVENT7 },
  { "MHPMEvent8", CSR_MHPMEVENT8 },
  { "MHPMEvent9", CSR_MHPMEVENT9 },
  { "MHPMEvent10", CSR_MHPMEVENT10 },
  { "MHPMEvent11", CSR_MHPMEVENT11 },
  { "MHPMEvent12", CSR_MHPMEVENT12 },
  { "MHPMEvent13", CSR_MHPMEVENT13 },
  { "MHPMEvent14", CSR_MHPMEVENT14 },
  { "MHPMEvent15", CSR_MHPMEVENT15 },
  { "MHPMEvent16", CSR_MHPMEVENT16 },
  { "MHPMEvent17", CSR_MHPMEVENT17 },
  { "MHPMEvent18", CSR_MHPMEVENT18 },
  { "MHPMEvent19", CSR_MHPMEVENT19 },
  { "MHPMEvent20", CSR_MHPMEVENT20 },
  { "MHPMEvent21", CSR_MHPMEVENT21 },
  { "MHPMEvent22", CSR_MHPMEVENT22 },
  { "MHPMEvent23", CSR_MHPMEVENT23 },
  { "MHPMEvent24", CSR_MHPMEVENT24 },
  { "MHPMEvent25", CSR_MHPMEVENT25 },
  { "MHPMEvent26", CSR_MHPMEVENT26 },
  { "MHPMEvent27", CSR_MHPMEVENT27 },
  { "MHPMEvent28", CSR_MHPMEVENT28 },
  { "MHPMEvent29", CSR_MHPMEVENT29 },
  { "MHPMEvent30", CSR_MHPMEVENT30 },
  { "MHPMEvent31", CSR_MHPMEVENT31 },
  { "MVendorID", CSR_MVENDORID },
  { "MArchID", CSR_MARCHID },
  { "MIMPID", CSR_MIMPID },
  { "MHartID", CSR_MHARTID },
  { "CycleH", CSR_CYCLEH },
  { "TimeH", CSR_TIMEH },
  { "InstRetH", CSR_INSTRETH },
  { "HPMCounter3H", CSR_HPMCOUNTER3H },
  { "HPMCounter4H", CSR_HPMCOUNTER4H },
  { "HPMCounter5H", CSR_HPMCOUNTER5H },
  { "HPMCounter6H", CSR_HPMCOUNTER6H },
  { "HPMCounter7H", CSR_HPMCOUNTER7H },
  { "HPMCounter8H", CSR_HPMCOUNTER8H },
  { "HPMCounter9H", CSR_HPMCOUNTER9H },
  { "HPMCounter10H", CSR_HPMCOUNTER10H },
  { "HPMCounter11H", CSR_HPMCOUNTER11H },
  { "HPMCounter12H", CSR_HPMCOUNTER12H },
  { "HPMCounter13H", CSR_HPMCOUNTER13H },
  { "HPMCounter14H", CSR_HPMCOUNTER14H },
  { "HPMCounter15H", CSR_HPMCOUNTER15H },
  { "HPMCounter16H", CSR_HPMCOUNTER16H },
  { "HPMCounter17H", CSR_HPMCOUNTER17H },
  { "HPMCounter18H", CSR_HPMCOUNTER18H },
  { "HPMCounter19H", CSR_HPMCOUNTER19H },
  { "HPMCounter20H", CSR_HPMCOUNTER20H },
  { "HPMCounter21H", CSR_HPMCOUNTER21H },
  { "HPMCounter22H", CSR_HPMCOUNTER22H },
  { "HPMCounter23H", CSR_HPMCOUNTER23H },
  { "HPMCounter24H", CSR_HPMCOUNTER24H },
  { "HPMCounter25H", CSR_HPMCOUNTER25H },
  { "HPMCounter26H", CSR_HPMCOUNTER26H },
  { "HPMCounter27H", CSR_HPMCOUNTER27H },
  { "HPMCounter28H", CSR_HPMCOUNTER28H },
  { "HPMCounter29H", CSR_HPMCOUNTER29H },
  { "HPMCounter30H", CSR_HPMCOUNTER30H },
  { "HPMCounter31H", CSR_HPMCOUNTER31H },
  { "MCycleH", CSR_MCYCLEH },
  { "MInstRetH", CSR_MINSTRETH },
  { "MHPMCounter3H", CSR_MHPMCOUNTER3H },
  { "MHPMCounter4H", CSR_MHPMCOUNTER4H },
  { "MHPMCounter5H", CSR_MHPMCOUNTER5H },
  { "MHPMCounter6H", CSR_MHPMCOUNTER6H },
  { "MHPMCounter7H", CSR_MHPMCOUNTER7H },
  { "MHPMCounter8H", CSR_MHPMCOUNTER8H },
  { "MHPMCounter9H", CSR_MHPMCOUNTER9H },
  { "MHPMCounter10H", CSR_MHPMCOUNTER10H },
  { "MHPMCounter11H", CSR_MHPMCOUNTER11H },
  { "MHPMCounter12H", CSR_MHPMCOUNTER12H },
  { "MHPMCounter13H", CSR_MHPMCOUNTER13H },
  { "MHPMCounter14H", CSR_MHPMCOUNTER14H },
  { "MHPMCounter15H", CSR_MHPMCOUNTER15H },
  { "MHPMCounter16H", CSR_MHPMCOUNTER16H },
  { "MHPMCounter17H", CSR_MHPMCOUNTER17H },
  { "MHPMCounter18H", CSR_MHPMCOUNTER18H },
  { "MHPMCounter19H", CSR_MHPMCOUNTER19H },
  { "MHPMCounter20H", CSR_MHPMCOUNTER20H },
  { "MHPMCounter21H", CSR_MHPMCOUNTER21H },
  { "MHPMCounter22H", CSR_MHPMCOUNTER22H },
  { "MHPMCounter23H", CSR_MHPMCOUNTER23H },
  { "MHPMCounter24H", CSR_MHPMCOUNTER24H },
  { "MHPMCounter25H", CSR_MHPMCOUNTER25H },
  { "MHPMCounter26H", CSR_MHPMCOUNTER26H },
  { "MHPMCounter27H", CSR_MHPMCOUNTER27H },
  { "MHPMCounter28H", CSR_MHPMCOUNTER28H },
  { "MHPMCounter29H", CSR_MHPMCOUNTER29H },
  { "MHPMCounter30H", CSR_MHPMCOUNTER30H },
  { "MHPMCounter31H", CSR_MHPMCOUNTER31H },
};

#endif // REGISTER_MAPPINGS_H
