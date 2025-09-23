/**
 * @file Issue14.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @author gary7530 (https://github.com/gary7530).
 *
 * @date 2025-09-23
 *
 * @brief Regression test for issue #14
 *
 * @see https://github.com/afpineda/NuS-NimBLE-Serial/issues/14
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <NimBLEDevice.h>
#include <NuSerial.hpp>

#define DEVICE_NAME "NusIssue14"

char nmeabuff[4096] = {
    "$GNGSA,A,1,05,07,13,14,15,21,22,30,,,,,1.4,0.7,1.2,1*33\n"
    "$GNGSA,A,1,04,10,11,12,19,21,27,29,,,,,1.4,0.7,1.2,3*33\n"
    "$GNGSA,A,1,14,21,24,26,,,,,,,,,1.4,0.7,1.2,4*33\n"
    "$GPGSV,3,1,12,05,68,268,47,07,37,057,45,08,01,050,35,09,04,097,26,0*6D\n"
    "$GPGSV,3,2,12,11,08,206,39,13,53,285,47,14,33,140,43,15,21,288,44,0*6B\n"
    "$GPGSV,3,3,12,18,12,326,38,21,61,176,46,22,21,153,41,30,68,071,47,0*6F\n"
    "$GAGSV,3,1,11,04,27,277,41,06,03,273,35,10,36,274,44,11,28,306,40,0*7B\n"
    "$GAGSV,3,2,11,12,33,245,40,19,80,343,41,21,30,133,41,27,33,070,42,0*70\n"
    "$GAGSV,3,3,11,29,42,083,43,30,07,026,31,33,07,196,38,,,,,0*45\n"
    "$GBGSV,2,1,07,09,14,049,34,10,04,080,30,14,49,303,46,21,46,119,46,0*7B\n"
    "$GBGSV,2,2,07,24,19,198,40,26,59,159,47,28,07,306,36,,,,,0*41\n"};

void setup()
{
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::getAdvertising()->setName(DEVICE_NAME);
    NuSerial.begin(115200);
}

void loop()
{
    NuSerial.printf("%s", nmeabuff);
    delay(5000);
}