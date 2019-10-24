#ifndef __SX1278_H
#define __SX1278_H

#include "main.h"
#include "spi.h"

// registers
#define REG_FIFO                 0x00
#define REG_OP_MODE              0x01
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08
#define REG_PA_CONFIG            0x09
#define REG_LR_OCP				 			 0X0b
#define REG_LNA                  0x0c
#define REG_FIFO_ADDR_PTR        0x0d
#define REG_FIFO_TX_BASE_ADDR    0x0e
#define REG_FIFO_RX_BASE_ADDR    0x0f
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_IRQ_FLAGS            0x12
#define REG_RX_NB_BYTES          0x13
#define REG_PKT_RSSI_VALUE       0x1a
#define REG_PKT_SNR_VALUE        0x1b
#define REG_MODEM_CONFIG_1       0x1d
#define REG_MODEM_CONFIG_2       0x1e
#define REG_PREAMBLE_MSB         0x20
#define REG_PREAMBLE_LSB         0x21
#define REG_PAYLOAD_LENGTH       0x22
#define REG_MODEM_CONFIG_3       0x26
#define REG_RSSI_WIDEBAND        0x2c
#define REG_DETECTION_OPTIMIZE   0x31
#define REG_DETECTION_THRESHOLD  0x37
#define REG_SYNC_WORD            0x39
#define REG_DIO_MAPPING_1        0x40
#define REG_VERSION              0x42
#define REG_PaDac				 				 0x4d//add REG_PaDac

// modes
#define MODE_LONG_RANGE_MODE     0x80
#define MODE_SLEEP               0x00
#define MODE_STDBY               0x01
#define MODE_TX                  0x03
#define MODE_RX_CONTINUOUS       0x05
#define MODE_RX_SINGLE           0x06

// PA config
#define PA_BOOST                 0x80
#define RFO                      0x70
// IRQ masks
#define IRQ_TX_DONE_MASK           0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20
#define IRQ_RX_DONE_MASK           0x40

#define MAX_PKT_LENGTH           255

#define PA_OUTPUT_PA_BOOST_PIN  1
#define PA_OUTPUT_RFO_PIN       0

#define ON 1
#define OFF 0

uint8_t LoRa_Init(double freq, uint8_t dbm, uint8_t sw);
void LoRa_Write(uint8_t addr, uint8_t data);
uint8_t LoRa_Read(uint8_t addr);
void LoRa_SetFreq(uint32_t freq);
void LoRa_SetTxPower(int8_t level);
void LoRa_SetSpreadingFactor(int8_t sf);
void LoRa_SetSignalBandwidth(uint32_t sbw);
void LoRa_SetSyncWord(uint8_t sw);
void LoRa_BeginPacket(uint8_t implicitHeader);
uint8_t LoRa_WritePacket(uint8_t* data, uint8_t len);
void LoRa_SendPacket(void);
uint8_t LoRa_ParsePacket(uint8_t size);

#endif
