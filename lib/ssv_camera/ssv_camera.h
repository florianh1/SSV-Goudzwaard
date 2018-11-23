/**
 * Control the VGA camera on the ESP32 board
*/

#include "stdint.h"

#define RT_VGA_BIT      0x01
#define RT_QVGA_BIT     0x02
#define RT_HVGA_BIT     0x03

#define CT_YUV_BIT      0x00
#define CT_RGB_BIT      0x01
#define CT_GRB_BIT      0x02
#define CT_RAWRGB_BIT   0x03
#define CT_MONO_BIT     0x04

/**
 * Generate the identification bytes of a video packet
 * 
 * @param uint8_t
 * @param uint8_t
 * @paran uint8_t
 * @param uint8_t
 * 
 * @return uint16_t
*/
uint16_t packetIdGen(uint8_t ID, uint8_t len, uint8_t rt, uint8_t ct);
