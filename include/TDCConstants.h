#ifndef TDCConstants_h
#define TDCConstants_h

#define TDC_NUM_CHANNELS      32
#define TDC_FIRMWARE_VERSION  0x8470DACE // for HPTDCv1.3

#define TDC_SETUP_REGISTER    0x8 // 0b1000
#define TDC_SETUP_BITS_NUM    647

#define TDC_CONTROL_REGISTER  0x9 // 0b1001
#define TDC_CONTROL_BITS_NUM  40

#define TDC_BS_REGISTER       0x0 // 0b0000
#define TDC_BS_BITS_NUM       83

#define TDC_STATUS_REGISTER   0xA // 0b1010
#define TDC_STATUS_BITS_NUM   62

#define CONFIG_START          0xCC // 204
#define CONFIG_REGISTER_NAME  0xE7 // 231
#define CONFIG_HPTDC_ID       0xFF // 255
#define CONFIG_START_STREAM   0xC7 // 199
#define CONFIG_STOP           0x3C // 60

#define VERIF_START           0xBC // 188
#define VERIF_REGISTER_NAME   0x1C // 28
#define VERIF_HPTDC_ID        0xCA // 202
#define VERIF_START_STREAM    0xCF // 207
#define VERIF_STOP            0x0F // 15

#define RO_START              0xDC // 220
#define RO_HPTDC_ID           0x84 // 132
#define RO_FIFO_SIZE          0x79 // 121
#define RO_NUM_BYTES          0x34 // 52
#define RO_START_STREAM       0x2C // 44
#define RO_STOP               0x5B // 91

#define FW_START              0xA3 // 163
#define FW_GET_VERSION        0x6B // 107
#define FW_STOP               0x29 // 41

#define RESET_WORD            0x99 // 153

#endif
