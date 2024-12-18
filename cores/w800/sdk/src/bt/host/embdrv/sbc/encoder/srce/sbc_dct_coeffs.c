/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  This file contains the coefficient table used for DCT computation in
 *  analysis.
 *
 ******************************************************************************/

#include "../include/sbc_encoder.h"
/*DCT coeff for 4 sub-band case.*/
#if (SBC_FAST_DCT == FALSE)
const SINT16 gas16AnalDCTcoeff4[] = {
    (SINT16)(0.7071 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(0.9239 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(0.0000 * 32768),
    (SINT16)(-0.3827 * 32768),

    (SINT16)(-0.7071 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(0.3827 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(-0.0000 * 32768),
    (SINT16)(0.9239 * 32768),

    (SINT16)(-0.7071 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(0.0000 * 32768),
    (SINT16)(-0.9239 * 32768),

    (SINT16)(0.7071 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(-0.0000 * 32768),
    (SINT16)(0.3827 * 32768)
};

/*DCT coeff for 8 sub-band case.*/
const SINT16 gas16AnalDCTcoeff8[] = {
    (SINT16)(0.7071 * 32768),
    (SINT16)(0.8315 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(0.9808 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(0.9808 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(0.8315 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(0.5556 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(0.1951 * 32768),
    (SINT16)(0.0000 * 32768),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(0.8315 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(0.8315 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(-0.0000 * 32768),
    (SINT16)(0.5556 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(0.9808 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(0.5556 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(0.5556 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(0.1951 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(0.8315 * 32768),
    (SINT16)(0.0000 * 32768),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(0.1951 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(0.1951 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(0.8315 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(-0.0000 * 32768),
    (SINT16)(0.9808 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(0.5556 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(0.5556 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(0.9808 * 32768),
    (SINT16)(0.0000 * 32768),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(0.8315 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(0.9808 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(0.9808 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(-0.0000 * 32768),
    (SINT16)(0.8315 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(0.1951 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(0.1951 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(0.1951 * 32768),
    (SINT16)(-0.7071 * 32768),
    (SINT16)(0.9808 * 32768),
    (SINT16)(-0.9239 * 32768),
    (SINT16)(0.5556 * 32768),
    (SINT16)(-0.0000 * 32768),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(1.0000 * 32767),
    (SINT16)(-0.9808 * 32768),
    (SINT16)(0.9239 * 32768),
    (SINT16)(-0.8315 * 32768),
    (SINT16)(0.7071 * 32768),
    (SINT16)(-0.5556 * 32768),
    (SINT16)(0.3827 * 32768),
    (SINT16)(-0.1951 * 32768),
    (SINT16)(-0.0000 * 32768),
    (SINT16)(0.1951 * 32768),
    (SINT16)(-0.3827 * 32768),
    (SINT16)(0.5556 * 32768)
};
#endif
