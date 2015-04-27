#ifndef TDCConstants_h
#define TDCConstants_h

#define TDC_NUM_CHANNELS      32
#define TDC_SETUP_BITS_NUM    647
#define TDC_CONTROL_BITS_NUM  40
#define TDC_BS_BITS_NUM       83
#define TDC_STATUS_BITS_NUM   62

#define CONFIG_START          0xCC // 204
#define CONFIG_REGISTER_NAME  0xE7 // 231
#define CONFIG_HPTDC_ID       0xFF // 255
#define CONFIG_START_STREAM   0xC7 // 199
#define CONFIG_STOP           0x3C // 60

#define VERIF_START           0xBC // 188
#define VERIG_REGISTER_NAME   0x1C // 28
#define VERIF_HPTDC_ID        0xCA // 202
#define VERIF_START_STREAM    0xCF // 207
#define VERIF_STOP            0x0F // 15

#define RO_START              0xDC // 220

#endif
