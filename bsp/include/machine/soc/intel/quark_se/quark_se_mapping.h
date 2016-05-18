/*
 * Copyright (c) 2015, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __QUARK_SE_MAPPING_H__
#define __QUARK_SE_MAPPING_H__

#define BASE_FLASH_ADDR 0x40000000
#define VERSION_HEADER_SIZE 0x30

#define EMBEDDED_OTP_FLASH_ID                           0
#define EMBEDDED_OTP_FLASH_NB_BLOCKS                    7
#define EMBEDDED_OTP_FLASH_BLOCK_SIZE                   1024

#define EMBEDDED_FLASH_ID                               1
#define EMBEDDED_FLASH_NB_BLOCKS                        192
#define EMBEDDED_FLASH_BLOCK_SIZE                       2048

#define SERIAL_FLASH_ID                                 2
#define SERIAL_FLASH_NB_BLOCKS                          512
#define SERIAL_FLASH_BLOCK_SIZE                         4096

#define NUMBER_OF_PARTITIONS                            10

#define FACTORY_RESET_NON_PERSISTENT                    0
#define FACTORY_RESET_PERSISTENT                        1

/* FACTORY_RESET_NON_PERSISTENT_PARTITION - 3 blocks */
#define FACTORY_RESET_NON_PERSISTENT_PARTITION_ID       0
#define FACTORY_RESET_NON_PERSISTENT_FLASH_ID           EMBEDDED_FLASH_ID
#define FACTORY_RESET_NON_PERSISTENT_START_BLOCK        180
#define FACTORY_RESET_NON_PERSISTENT_END_BLOCK          ( \
		FACTORY_RESET_NON_PERSISTENT_START_BLOCK + 2)
#define FACTORY_RESET_NON_PERSISTENT_NB_BLOCKS          ( \
		FACTORY_RESET_NON_PERSISTENT_END_BLOCK - \
		FACTORY_RESET_NON_PERSISTENT_START_BLOCK + 1)

/* FACTORY_RESET_PERSISTENT_PARTITION - 3 blocks */
#define FACTORY_RESET_PERSISTENT_PARTITION_ID           1
#define FACTORY_RESET_PERSISTENT_FLASH_ID               EMBEDDED_FLASH_ID
#define FACTORY_RESET_PERSISTENT_START_BLOCK            ( \
		FACTORY_RESET_NON_PERSISTENT_END_BLOCK + 1)
#define FACTORY_RESET_PERSISTENT_END_BLOCK              ( \
		FACTORY_RESET_PERSISTENT_START_BLOCK + 2)
#define FACTORY_RESET_PERSISTENT_NB_BLOCKS              ( \
		FACTORY_RESET_PERSISTENT_END_BLOCK - \
		FACTORY_RESET_PERSISTENT_START_BLOCK + 1)

/* APPLICATION_DATA_PARTITION - 4 blocks */
#define APPLICATION_DATA_PARTITION_ID                   2
#define APPLICATION_DATA_FLASH_ID                       EMBEDDED_FLASH_ID
#define APPLICATION_DATA_START_BLOCK                    ( \
		FACTORY_RESET_PERSISTENT_END_BLOCK + 1)
#define APPLICATION_DATA_END_BLOCK                      ( \
		APPLICATION_DATA_START_BLOCK + 3)
#define APPLICATION_DATA_NB_BLOCKS                      ( \
		APPLICATION_DATA_END_BLOCK - APPLICATION_DATA_START_BLOCK + 1)

/* FACTORY_SETTINGS_PARTITION - 2 blocks */
#define FACTORY_SETTINGS_PARTITION_ID                   4
#define FACTORY_SETTINGS_FLASH_ID                       EMBEDDED_FLASH_ID
#define FACTORY_SETTINGS_START_BLOCK                    ( \
		APPLICATION_DATA_END_BLOCK + 1)
#define FACTORY_SETTINGS_END_BLOCK                      ( \
		FACTORY_SETTINGS_START_BLOCK + 1)

/* Partition used for FOTA - 253 blocks = 1012 kB */
#define SPI_FOTA_PARTITION_ID                           5
#define SPI_FOTA_FLASH_ID                               SERIAL_FLASH_ID
#define SPI_FOTA_START_BLOCK                            0
#define SPI_FOTA_END_BLOCK                              252
#define SPI_FOTA_NB_BLOCKS                              (SPI_FOTA_END_BLOCK - \
							 SPI_FOTA_START_BLOCK +	\
							 1)

/* Partition used for LOG 253 blocks = 1012 kB same as FOTA partition */
#define SPI_LOG_PARTITION_ID                           SPI_FOTA_PARTITION_ID
#define SPI_LOG_FLASH_ID                               SPI_FOTA_FLASH_ID
#define SPI_LOG_START_BLOCK                            SPI_FOTA_START_BLOCK
#define SPI_LOG_END_BLOCK                              SPI_FOTA_END_BLOCK
#define SPI_LOG_NB_BLOCKS                              SPI_FOTA_NB_BLOCKS

/* Partition used for Activity data storage - 256 blocks = 1 MB */
#define SPI_APPLICATION_DATA_PARTITION_ID               6
#define SPI_APPLICATION_DATA_FLASH_ID                   SERIAL_FLASH_ID
#define SPI_APPLICATION_DATA_START_BLOCK                (SPI_FOTA_END_BLOCK + 1)
#define SPI_APPLICATION_DATA_END_BLOCK                  ( \
		SPI_APPLICATION_DATA_START_BLOCK + 255)
#define SPI_APPLICATION_DATA_NB_BLOCKS                  ( \
		SPI_APPLICATION_DATA_END_BLOCK - \
		SPI_APPLICATION_DATA_START_BLOCK + 1)

/* Partition used for System Event storage - 3 blocks = 12 kB */
#define SPI_SYSTEM_EVENT_PARTITION_ID                   7
#define SPI_SYSTEM_EVENT_FLASH_ID                       SERIAL_FLASH_ID
#define SPI_SYSTEM_EVENT_START_BLOCK                    ( \
		SPI_APPLICATION_DATA_END_BLOCK + 1)
#define SPI_SYSTEM_EVENT_END_BLOCK                      ( \
		SPI_SYSTEM_EVENT_START_BLOCK + 2)
#define SPI_SYSTEM_EVENT_NB_BLOCKS                      ( \
		SPI_SYSTEM_EVENT_END_BLOCK - SPI_SYSTEM_EVENT_START_BLOCK + 1)

#define PERSIST_PROPS_BLKS_NB                           ( \
		FACTORY_RESET_PERSISTENT_END_BLOCK - \
		FACTORY_RESET_PERSISTENT_START_BLOCK + 1)
#define NON_PERSIST_PROPS_BLKS_NB                       ( \
		FACTORY_RESET_NON_PERSISTENT_END_BLOCK - \
		FACTORY_RESET_NON_PERSISTENT_START_BLOCK + 1)
#define TOTAL_PROP_BLKS_NB                              (PERSIST_PROPS_BLKS_NB \
							 + \
							 NON_PERSIST_PROPS_BLKS_NB)


/** Data shared between ARC and QRK */
#define ARC_QRK_SHARED_DATA     0xa8000000

/** USB Driver static data shared between
 * QRK Bootloader and QRK App
 */
#define BOOTLOADER_SHARED_DATA  0xa8000400

/* DEBUGPANIC_PARTITION - 2 blocks */
#define DEBUGPANIC_PARTITION_ID                         3
#define DEBUGPANIC_FLASH_ID                             EMBEDDED_FLASH_ID
#define DEBUGPANIC_START_BLOCK                          30
#define DEBUGPANIC_END_BLOCK                            (DEBUGPANIC_START_BLOCK	\
							 + 1)
#define DEBUGPANIC_NB_BLOCKS                            (DEBUGPANIC_END_BLOCK -	\
							 DEBUGPANIC_START_BLOCK	\
							 + 1)

#ifndef QUARK_START_PAGE
#define QUARK_START_PAGE 32
#endif

#ifndef QUARK_NB_PAGE
#define QUARK_NB_PAGE 72
#endif

#define QUARK_SIZE (QUARK_NB_PAGE * EMBEDDED_FLASH_BLOCK_SIZE)
#define QUARK_FLASH_START_ADDR (BASE_FLASH_ADDR + \
				(QUARK_START_PAGE * EMBEDDED_FLASH_BLOCK_SIZE))
#define QUARK_RAM_START_ADDR 0xA8000c00
#define QUARK_RAM_SIZE  53  /* 53k */
#define QUARK_DCCM_START_ADDR 0x80000000
#define QUARK_DCCM_SIZE       (8 * 1024)

#define ARC_START_PAGE (QUARK_NB_PAGE + QUARK_START_PAGE)
#define ARC_NB_PAGE (148 - QUARK_NB_PAGE)
#define ARC_SIZE (ARC_NB_PAGE * EMBEDDED_FLASH_BLOCK_SIZE)
#define ARC_FLASH_START_ADDR (BASE_FLASH_ADDR +	\
			      (ARC_START_PAGE * EMBEDDED_FLASH_BLOCK_SIZE))
#define ARC_RAM_START_ADDR 0xa800e000
#define ARC_RAM_SIZE CONFIG_QUARK_SE_ARC_RAM_SIZE
#define ARC_DCCM_START_ADDR 0x80000000
#define ARC_DCCM_SIZE       CONFIG_QUARK_SE_ARC_DCCM_SIZE

#endif /* __QUARK_SE_MAPPING_H__ */
