#ifndef APERTURE_TUNING_H
#define APERTURE_TUNING_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_CHANNELS_RX 6 // 12
#define NUM_CHANNELS_TX 1
#define NUM_CHANNELS (NUM_CHANNELS_RX + NUM_CHANNELS_TX)




/* IO Expander addresses / registers (adjust if needed) */
#define IO_EXP_ADDR1 0x22
#define IO_EXP_ADDR2 0x23
#define IO_EXP_OUTPUTS_REG 0x84
#define IO_EXP_CONFIG_REG  0x8C
#define IO_EXP_INVERT_REG  0x88

/* Bit offsets and masks */
#define MATCHING_OFFSET 0
#define TUNING_OFFSET   4
#define DETUNE_ENABLE_OFFSET 11

/* Sizes */
#define MATCHING_MAX_BITS 4
#define TUNING_MAX_BITS   7

/* IO expander bus IDs */
#define IO_EXP_BUS0 0
#define IO_EXP_BUS1 1
#define IO_EXP_BUS2 2
#define IOEXP_TX_BUS 3

/* channel active flags */
#define ACTIVE_CHANNEL 1
#define INACTIVE_CHANNEL 0

/* precompute sets limit - change as needed */
#ifndef MAX_PRESETS
#define MAX_PRESETS 16
#endif

/* number of IO expanders in your mapping - adjust if you add/remove */
#ifndef NUM_IO_EXPANDERS
#define NUM_IO_EXPANDERS 7
#endif

/* structure for a single channel config (tuning, matching, detune_enable) */
typedef struct {
    uint8_t tuning;        /* 0..127 (7 bits) */
    uint8_t matching;      /* 0..15 (4 bits) */
    uint8_t detune_enable; /* 0..1  (1 bit)  */
} aperture_channel_config_t;

/* container for a single precomputed IO expander payload (3 bytes) */
typedef struct {
    uint8_t bytes[3];
} ioexp_payload_t;

/* public API */

/* Initialize controller (set up expanders, i2c, etc) */
int atc_init(void);

/* Provide mapping arrays (see default mapping in C file) */
int atc_precompute_sets(const aperture_channel_config_t presets[][NUM_CHANNELS],
                        uint8_t num_presets);

/* Apply a given preset index synchronously (blocking I2C writes) */
int atc_apply_preset_blocking(uint8_t preset_index);

/* Queue a preset to be applied asynchronously (non-blocking) - driver dependent */
int atc_apply_preset_async(uint8_t preset_index);

/* Utility: build a single channel encode */
uint16_t atc_encode_channel_config(const aperture_channel_config_t *cfg);

/* Utility: decode 12-bit channel value to struct */
void atc_decode_channel_config(uint16_t encoded, aperture_channel_config_t *out);

/* Optional: get pointer to precomputed payload for a given preset and expander */
const ioexp_payload_t* atc_get_payload_ptr(uint8_t preset_index, uint8_t expander_index);

/* Number of expanders used */
uint8_t atc_num_io_expanders(void);

#endif /* APERTURE_TUNING_H */
