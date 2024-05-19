/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

s16 _svm_stereo_mono;
int _svm_vab_not_send_size;
SpuReverbAttr _svm_rattr;
char _svm_vab_used[16];
char _SsVmMaxVoice;
s16 _svm_vab_count;
s16 kMaxPrograms;
libsnd2_svm_t _svm_cur;
s16 _svm_damper;
u8 _svm_auto_kof_mode;
VabHdr *_svm_vab_vh[16];
ProgAtr *_svm_vab_pg[16];
VagAtr *_svm_vab_tn[16];
int _svm_vab_start[16];
int _svm_vab_total[16];
VabHdr *_svm_vh;
ProgAtr *_svm_pg;
VagAtr *_svm_tn;
