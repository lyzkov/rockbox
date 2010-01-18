/* This config file is for all simulators, and is used in addition to the
   target specific configs */

#undef CONFIG_CPU

#undef HAVE_FMADC

#undef NEED_ATA_POWER_BATT_MEASURE

#undef CONFIG_I2C

#undef HAVE_PCM_DMA_ADDRESS

#undef FLASH_SIZE

#undef CPU_FREQ

#undef HAVE_ATA_POWER_OFF

#undef CONFIG_LCD

#undef CONFIG_LED

#undef ROM_START

#undef FIRMWARE_OFFSET_FILE_LENGTH
#undef FIRMWARE_OFFSET_FILE_CRC
#undef FIRMWARE_OFFSET_FILE_DATA

#undef AMS_OF_SIZE

#undef HAVE_MULTIDRIVE
#undef NUM_DRIVES
#undef HAVE_HOTSWAP
#undef HAVE_HOTSWAP_STORAGE_AS_MAIN

#undef CONFIG_STORAGE_MULTI
#undef CONFIG_STORAGE
#undef NUM_DRIVES

#define CONFIG_STORAGE_MULTI
#define CONFIG_STORAGE STORAGE_RAMDISK
#define NUM_DRIVES 1

#undef CONFIG_USBOTG

#undef USB_HANDLED_BY_OF

#undef USB_NUM_ENDPOINTS
#define USB_NUM_ENDPOINTS    8
/* with the simulator, the previous definition can't be dangerous so keep it if already defined */
#ifndef USB_DEVBSS_ATTR
#define USB_DEVBSS_ATTR
#endif /* USB_DEVBSS_ATTR */
#define USB_STATUS_BY_EVENT
#define USB_DETECT_BY_DRV
#define USB_USE_RAMDISK

#undef HAVE_ADJUSTABLE_CPU_FREQ

#undef MI4_FORMAT
#undef BOOTFILE_EXT
#undef BOOTFILE
#undef BOOTDIR

#undef BOOTLOADER_ENTRYPOINT

#undef FLASH_ENTRYPOINT

#undef FLASH_MAGIC

#undef HAVE_EEPROM
#undef HAVE_EEPROM_SETTINGS

#undef HAVE_HARDWARE_BEEP

#undef HAVE_POWEROFF_WHILE_CHARGING

#undef INCLUDE_TIMEOUT_API

#undef HAVE_FLASHED_ROCKBOX

#undef IPOD_ACCESSORY_PROTOCOL

#undef HAVE_WHEEL_POSITION

#undef HAVE_LCD_MODES

#undef HAVE_SPEAKER

