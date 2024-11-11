// Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5.
// See LICENSE for details.

#pragma once
#include <Headers/kern_util.hpp>

constexpr UInt32 mmPCIE_INDEX = 0xC;
constexpr UInt32 mmPCIE_DATA = 0xD;
constexpr UInt32 mmPCIE_INDEX2 = 0xE;
constexpr UInt32 mmPCIE_DATA2 = 0xF;

constexpr UInt32 mmSMC_IND_INDEX_0 = 0x80;
constexpr UInt32 mmSMC_IND_DATA_0 = 0x81;
constexpr UInt32 mmSMC_IND_INDEX_11 = 0x1AC;
constexpr UInt32 mmSMC_IND_DATA_11 = 0x1AD;

constexpr UInt32 ixCG_MULT_THERMAL_STATUS = 0xC0300014;

constexpr UInt32 THM_BASE = 0x16600;

constexpr UInt32 mmTHM_TCON_CUR_TMP = 0;
constexpr UInt32 CUR_TEMP_RANGE_SEL = 0x80000;
#define GET_TCON_CUR_TEMP(v) (((v) & 0xFFE00000) >> 21)

constexpr UInt32 mmCG_MULT_THERMAL_STATUS_THM9 = 0x5A;
constexpr UInt32 mmCG_MULT_THERMAL_STATUS_THM11 = 0x5F;
#define GET_THERMAL_STATUS_CTF_TEMP(v) (((v) & 0x3FE00) >> 9)

constexpr UInt32 AMDGPU_MAX_USEC_TIMEOUT = 100000;

constexpr UInt32 MP_BASE_SMU9 = 0x16000;
constexpr UInt32 mmMP1_SMN_C2PMSG_66 = 0x282;
constexpr UInt32 mmMP1_SMN_C2PMSG_82 = 0x292;
constexpr UInt32 mmMP1_SMN_C2PMSG_90 = 0x29A;

constexpr UInt32 mmSMC_MESSAGE_0_SMU7 = 0x94;
constexpr UInt32 mmSMC_RESP_0_SMU7 = 0x95;
constexpr UInt32 SMC_RESP_0_MASK_SMU7 = 0xFFFF;
constexpr UInt32 mmSMC_MSG_ARG_0_SMU7 = 0xA4;

constexpr UInt32 ixSMU_PM_STATUS_95 = 0x3FF7C;

constexpr UInt32 PPSMC_MSG_PmStatusLogStart_SMU7 = 0x170;
constexpr UInt32 PPSMC_MSG_PmStatusLogSample_SMU7 = 0x171;
constexpr UInt32 PPSMC_MSG_GetCurrPkgPwr_SMU7 = 0x282;

constexpr UInt32 PPSMC_MSG_GetCurrPkgPwr_SMU9 = 0x61;
