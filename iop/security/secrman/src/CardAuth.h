extern void ResetMcHandlers(void);
extern void SetMcCommandHandler(McCommandHandler_t handler);
extern McCommandHandler_t GetMcCommandHandler(void);
extern void SetMcDevIDHandler(McDevIDHandler_t handler);
extern McDevIDHandler_t GetMcDevIDHandler(void); // unused.
extern int McDeviceIDToCNum(int port, int slot);
extern int SendMcCommand(int port, int slot, sio2_transfer_data_t *sio2packet);
extern int card_auth(int port, int slot, int arg3, int command);
extern unsigned char calculate_sio2_buffer_checksum(const void *buffer, unsigned int length);
extern int card_auth_write(int port, int slot, const void *buffer, int arg3, int command);
extern int card_auth_read(int port, int slot, void *buffer, int arg3, int command);

extern int card_auth_60(int port, int slot);
extern int card_auth_key_change(int port, int slot, int command);
extern int card_auth2(int port, int slot, int arg3, int command);
