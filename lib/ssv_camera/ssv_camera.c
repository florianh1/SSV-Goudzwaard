#include "stdint.h"

#include <ssv_camera.h>

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
uint16_t packetIdGen(uint8_t ID, uint8_t len, uint8_t rt, uint8_t ct)
{
    uint16_t packetID = 0x0000;
    
    // ID ID ID ID ID RT RT res len len len len len CT CT CT 
    // 15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0
    
    packetID = (((uint16_t)(ID << 3) + (uint16_t)(rt << 1)) << 8); // byte 1
    packetID += ((uint16_t)(len << 3) + (uint16_t)(ct)); // byte 2
    
    return packetID;
}
