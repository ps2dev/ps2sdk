extern int SendMechaCommand(int command, const void *input, unsigned short int length, void *output);

extern int mechacon_auth_80(int cnum);
extern int mechacon_auth_81(int cnum);
extern int mechacon_auth_82(const void *buffer, const void *buffer2);
extern int mechacon_auth_83(const void *buffer);
extern int mechacon_auth_84(void *buffer, void *buffer2);
extern int mechacon_auth_85(void *buffer, void *buffer2);
extern int mechacon_auth_86(void *buffer, void *buffer2);
extern int mechacon_auth_87(void *buffer);
extern int mechacon_auth_88(void);

extern int write_HD_start(unsigned char mode, int cnum, int arg2, unsigned short int HeaderLength);
extern int write_data(const void *buffer, unsigned short int length);
extern int pol_cal_cmplt(void);
extern int func_00001c98(unsigned short int size);
extern int get_BIT_length(unsigned short int *BitLength);
extern int func_00001b00(void *data, unsigned short int length);
extern int mechacon_set_block_size(unsigned short int size);
extern int _PreEncryptKbit1(void *kbit1); // request first  half of pre-encrypted kbit
extern int _PreEncryptKbit2(void *kbit2); // request second half of pre-encrypted kbit
extern int _PreEncryptKc1(void *kc1); // request first  half of pre-encrypted kc
extern int _PreEncryptKc2(void *kc2); // request second half of pre-encrypted kc
extern int func_00001ed8(void *icvps2);
