int SendMechaCommand(int command, const void *input, unsigned short int length, void *output);

int mechacon_auth_80(int cnum);
int mechacon_auth_81(int cnum);
int mechacon_auth_82(const void *buffer, const void *buffer2);
int mechacon_auth_83(const void *buffer);
int mechacon_auth_84(void *buffer, void *buffer2);
int mechacon_auth_85(void *buffer, void *buffer2);
int mechacon_auth_86(void *buffer, void *buffer2);
int mechacon_auth_87(void *buffer);
int mechacon_auth_88(void);

int write_HD_start(unsigned char mode, int cnum, int arg2, unsigned short int HeaderLength);
int write_data(const void *buffer, unsigned short int length);
int pol_cal_cmplt(void);
int func_00001c98(unsigned short int size);
int get_BIT_length(unsigned short int *BitLength);
int func_00001b00(void *data, unsigned short int length);
int mechacon_set_block_size(unsigned short int size);
int func_00001ce8(void *kbit1);
int func_00001d64(void *kbit2);
int func_00001de0(void *kc1);
int func_00001e5c(void *kc2);
int func_00001ed8(void *icvps2);
