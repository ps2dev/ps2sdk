#include <stdio.h>
#include <sysclib.h>
#include "secrman.h"
#include "main.h"
#include "CardAuth.h"
#include "MechaAuth.h"
#include "DongleAuth.h"


int SecrAuthDongle(int port,int slot,int cnum)
{
  int x;
  u8 CardIV[8];
  u8 CardMaterial[8];
  u8 CardNonce[8];
  u8 MechaChallenge3[7];
  u8 MechaChallenge2[5];
  u8 auStack_44[4];
  u8 MechaChallenge1[8];
  u8 CardResponse1[8];
  u8 CardResponse2[8];
  u8 CardResponse3[8];
  
  if (GetMcCommandHandler() == NULL) {
    _arcade_printf("mcCommand isn\'t assigned\n");
  }
  else {
    memset(MechaChallenge2, 0, sizeof(MechaChallenge2));
    _arcade_printf("SecrAuthDongle start\n");
    x = card_auth_60(port,slot);
    if (x != 0) {
      _arcade_printf("dongle auth 0x60\n");
      x = mechacon_auth_80(cnum);
      if (x != 0) {
        _arcade_printf("mechacon auth 0x80\n");
        x = card_auth(port,slot,0xf0,0);
        if (x == 0) {
          card_auth_60(port,slot);
        }
        else {
          _arcade_printf("dongle auth 0x00\n");
          x = mechacon_auth_81(cnum | 0x40);
          if (x == 0) {
            card_auth_60(port,slot);
          }
          else {
            _arcade_printf("mechacon auth 0x81\n");
            x = card_auth_read(port,slot,CardIV,0xf0,1);
            if (x != 0) {
              _arcade_printf("dongle auth 0x01\n");
              x = card_auth_read(port,slot,CardMaterial,0xf0,2);
              if (x != 0) {
                _arcade_printf("dongle auth 0x02\n");
                x = mechacon_auth_82(CardIV,CardMaterial);
                if (x != 0) {
                  _arcade_printf("mechacon auth 0x82\n");
                  x = card_auth(port,slot,0xf0,3);
                  if (x != 0) {
                    _arcade_printf("dongle auth 0x03\n");
                    x = card_auth_read(port,slot,CardNonce,0xf0,4);
                    if (x != 0) {
                      _arcade_printf("dongle auth 0x04\n");
                      x = mechacon_auth_83(CardNonce);
                      if (x != 0) {
                        _arcade_printf("mechacon auth 0x83\n");
                        x = card_auth(port,slot,0xf0,5);
                        if (x != 0) {
                          _arcade_printf("dongle auth 0x05\n");
                          x = mechacon_auth_8F();
                          if (x != 0) {
                            _arcade_printf("mechacon auth 0x8f\n");
                            x = mechacon_auth_84(MechaChallenge1,MechaChallenge2 + 1);
                            if (x != 0) {
                              _arcade_printf("mechacon auth 0x84\n");
                              x = mechacon_auth_85(auStack_44,MechaChallenge3);
                              if (x != 0) {
                                _arcade_printf("mechacon auth 0x85\n");
                                x = card_auth_write(port,slot,MechaChallenge3,0xf0,6);
                                if (x != 0) {
                                  _arcade_printf("dongle auth 0x06\n");
                                  x = card_auth_write(port,slot,MechaChallenge2 + 1,0xf0,7);
                                  if (x != 0) {
                                    _arcade_printf("dongle auth 0x07\n");
                                    x = card_auth(port,slot,0xf0,8);
                                    if (x != 0) {
                                      _arcade_printf("dongle auth 0x08\n");
                                      x = card_auth2(port,slot,0xf0,9);
                                      if (x != 0) {
                                        _arcade_printf("dongle auth 0x09\n");
                                        x = card_auth(port,slot,0xf0,10);
                                        if (x != 0) {
                                          _arcade_printf(0x3ff8);
                                          x = card_auth_write(port,slot,MechaChallenge1,0xf0,0xb);
                                          if (x != 0) {
                                            _arcade_printf("dongle auth 0x0b\n");
                                            x = card_auth(port,slot,0xf0,0xc);
                                            if (x != 0) {
                                              _arcade_printf("dongle auth 0x0c\n");
                                              x = card_auth2(port,slot,0xf0,0xd);
                                              if (x != 0) {
                                                _arcade_printf("dongle auth 0x0d\n");
                                                x = card_auth(port,slot,0xf0,0xe);
                                                if (x != 0) {
                                                  _arcade_printf("dongle auth 0x0e\n");
                                                  x = card_auth_read(port,slot,CardResponse1,0xf0,
                                                                      0xf);
                                                  if (x != 0) {
                                                    _arcade_printf("dongle auth 0x0f\n");
                                                    x = card_auth(port,slot,0xf0,0x10);
                                                    if (x != 0) {
                                                      _arcade_printf("dongle auth 0x10\n");
                                                      x = card_auth_read(port,slot,CardResponse2,0xf0,0x11);
                                                      if (x != 0) {
                                                        _arcade_printf("dongle auth 0x11\n");
                                                        x = mechacon_auth_86(CardResponse1,CardResponse2);
                                                        if (x != 0) {
                                                          _arcade_printf("mechacon auth 0x86\n");
                                                          x = card_auth(port,slot,0xf0,0x12);
                                                          if (x != 0) {
                                                            _arcade_printf("dongle auth 0x12\n");
                                                            x = card_auth_read(port,slot,CardResponse3,0xf0,0x13);
                                                            if (x != 0) {
                                                              _arcade_printf("dongle auth 0x13\n");
                                                              x = mechacon_auth_87(CardResponse3);
                                                              if (x != 0) {
                                                                _arcade_printf("mechacon auth 0x87\n");
                                                                x = mechacon_auth_8F();
                                                                if (x != 0) {
                                                                  _arcade_printf("mechacon auth 0x8f\n");
                                                                  x = card_auth(port,slot,0xf0,0x14);
                                                                  if (x != 0) {
                                                                    _arcade_printf("dongle auth 0x14\n");
                                                                    x = mechacon_auth_88();
                                                                    if (x != 0) {
                                                                      _arcade_printf("mechacon auth 0x88\n");
                                                                      return 1;
                                                                    }
                                                                  }
                                                                }
                                                              }
                                                            }
                                                          }
                                                        }
                                                      }
                                                    }
                                                  }
                                                }
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            card_auth_60(port,slot);
            mechacon_auth_80(cnum);
          }
        }
      }
    }
  }
  return 0;
}

