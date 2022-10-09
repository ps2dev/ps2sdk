void ResetMcHandlers(void);
void SetMcCommandHandler(McCommandHandler_t handler);
McCommandHandler_t GetMcCommandHandler(void);
void SetMcDevIDHandler(McDevIDHandler_t handler);
McDevIDHandler_t GetMcDevIDHandler(void); // unused.
int McDeviceIDToCNum(int port, int slot);
int SendMcCommand(int port, int slot, sio2_transfer_data_t *sio2packet);
int card_auth(int port, int slot, int arg3, int command);
unsigned char calculate_sio2_buffer_checksum(const void *buffer, unsigned int length);
int card_auth_write(int port, int slot, const void *buffer, int arg3, int command);
int card_auth_read(int port, int slot, void *buffer, int arg3, int command);

int card_auth_60(int port, int slot);
int card_auth_key_change(int port, int slot, int command);
int card_auth2(int port, int slot, int arg3, int command);
