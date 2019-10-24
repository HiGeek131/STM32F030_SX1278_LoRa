#include <sx1278.h>

/*

PA5 -- LoRa_SCK
PA6 -- LoRa_MISO
PA7 -- LoRa_MOSI
PB1 -- LoRa_CS
PA9 -- LoRa_REST
PA10 -- LoRa_IRQ(DIO0)

*/

uint8_t _implicitHeaderMode;
uint8_t _packetIndex;
uint8_t _frequency;

uint8_t LoRa_singleTransfer(uint8_t addr, uint8_t data)
{
	HAL_GPIO_WritePin(LoRa_CS_GPIO_Port, LoRa_CS_Pin, GPIO_PIN_RESET);
	uint8_t buff_send[2];
	uint8_t buff_read[2];
	buff_send[0] = addr;
	buff_send[1] = data;
	HAL_SPI_TransmitReceive(&hspi1, buff_send, buff_read, 2, 0xff);
	HAL_GPIO_WritePin(LoRa_CS_GPIO_Port, LoRa_CS_Pin, GPIO_PIN_SET);
	return buff_read[1];
}

void LoRa_Write(uint8_t addr, uint8_t data)
{
	LoRa_singleTransfer(addr | 0x80, data);
}

uint8_t LoRa_Read(uint8_t addr)
{
	return LoRa_singleTransfer(addr & 0x7f, 0x00);
}

void LoRa_ImplicitHeaderMode()
{
	_implicitHeaderMode = 1;
	LoRa_Write(REG_MODEM_CONFIG_1, LoRa_Read(REG_MODEM_CONFIG_1) | 0x01);
}

void LoRa_ExplicitHeaderMode()
{
	_implicitHeaderMode = 0;
	LoRa_Write(REG_MODEM_CONFIG_1, LoRa_Read(REG_MODEM_CONFIG_1) & 0xfe);
}

void LoRa_Sleep()
{
	LoRa_Write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

void LoRa_Stdby()
{
	LoRa_Write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void LoRa_SetFreq(uint32_t freq)
{
	uint64_t frf = ((uint64_t)freq << 19) / 32000000;
	LoRa_Write(REG_FRF_MSB, (uint8_t)(frf >> 16));
	LoRa_Write(REG_FRF_MID, (uint8_t)(frf >> 8));
	LoRa_Write(REG_FRF_LSB, (uint8_t)(frf));
}

void LoRa_SetTxPower(int8_t level)
{
	if(level > 17)
	{
		if(level < 5)
			level = 5;
		LoRa_Write(REG_LR_OCP, 0x3f);
		LoRa_Write(REG_PaDac, 0x87);
		LoRa_Write(REG_PA_CONFIG, PA_BOOST | (level - 5));
	}
	else if(level > 14)
	{
		if(level < 2)
			level = 2;
		LoRa_Write(REG_PaDac, 0x87);
		LoRa_Write(REG_PA_CONFIG, PA_BOOST | (level - 2));
	}
	else
	{
		LoRa_Write(REG_PaDac, 0x84);
		LoRa_Write(REG_PA_CONFIG, RFO | (level + 1));
	}
}

void LoRa_SetSpreadingFactor(int8_t sf)
{
	if(6 == sf)
	{
		LoRa_Write(REG_DETECTION_OPTIMIZE, 0xc5);
		LoRa_Write(REG_DETECTION_THRESHOLD, 0x0c);
	}
	else
	{
		LoRa_Write(REG_DETECTION_OPTIMIZE, 0xc3);
		LoRa_Write(REG_DETECTION_THRESHOLD, 0x0a);
	}
	LoRa_Write(REG_MODEM_CONFIG_2, (LoRa_Read(REG_MODEM_CONFIG_2) & 0x0f) | (sf << 4));
}

void LoRa_SetSignalBandwidth(uint32_t sbw)
{
	uint8_t bw;
	
	if(sbw <= 7.8E3) bw = 0;
	else if(sbw <= 10.4E3) bw = 1;
	else if(sbw <= 15.6E3) bw = 2;
	else if(sbw <= 20.8E3) bw = 3;
	else if(sbw <= 31.25E3) bw = 4;
	else if(sbw <= 41.7E3) bw = 5;
	else if(sbw <= 62.5E3) bw = 6;
	else if(sbw <= 125E3) bw = 7;
	else if(sbw <= 250E3) bw = 8;
	else bw = 9;
	LoRa_Write(REG_MODEM_CONFIG_1, (LoRa_Read(REG_MODEM_CONFIG_1) & 0x0f) | (bw << 4));
}

void LoRa_SetSyncWord(uint8_t sw)
{
	LoRa_Write(REG_SYNC_WORD, sw);
}

void LoRa_SetCrc(uint8_t status)
{
	if(status)
		LoRa_Write(REG_MODEM_CONFIG_2, LoRa_Read(REG_MODEM_CONFIG_2) | 0x04);
	else
		LoRa_Write(REG_MODEM_CONFIG_2, LoRa_Read(REG_MODEM_CONFIG_2) & 0xfb);
}

void LoRa_BeginPacket(uint8_t implicitHeader)
{
	LoRa_Stdby();
	if(implicitHeader)
		LoRa_ImplicitHeaderMode();
	else
		LoRa_ExplicitHeaderMode();
	LoRa_Write(REG_FIFO_ADDR_PTR, 0);
	LoRa_Write(REG_PAYLOAD_LENGTH, 0);
}

uint8_t LoRa_WritePacket(uint8_t* data, uint8_t len)
{
	uint8_t currentLength = LoRa_Read(REG_PAYLOAD_LENGTH);
	if((currentLength + len) > MAX_PKT_LENGTH)
	{
		len = MAX_PKT_LENGTH - currentLength;
	}
	for(uint8_t i = 0; i < len; i ++)
	{
		LoRa_Write(REG_FIFO, data[i]);
	}
	LoRa_Write(REG_PAYLOAD_LENGTH, currentLength + len);
	return len;
}

void LoRa_SendPacket()
{
	LoRa_Write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
	while((LoRa_Read(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0)
		HAL_Delay(2);
	LoRa_Write(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
}

uint8_t LoRa_ParsePacket(uint8_t size)
{
	uint8_t packetLength = 0;
	uint8_t irqFlags = LoRa_Read(REG_IRQ_FLAGS);
	
	if(size > 0)
	{
		LoRa_ImplicitHeaderMode();
		LoRa_Write(REG_PAYLOAD_LENGTH, size);
	}
	else
	{
		LoRa_ExplicitHeaderMode();
	}
	
	LoRa_Write(REG_IRQ_FLAGS, irqFlags);
	
	if((irqFlags & IRQ_RX_DONE_MASK) && (irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0)
	{
		_packetIndex = 0;
		if(_implicitHeaderMode)
		{
			packetLength = LoRa_Read(REG_PAYLOAD_LENGTH);
		}
		else
		{
			packetLength = LoRa_Read(REG_RX_NB_BYTES);
		}
		LoRa_Write(REG_FIFO_ADDR_PTR, LoRa_Read(REG_FIFO_RX_CURRENT_ADDR));
		LoRa_Stdby();
	}
	else if(LoRa_Read(REG_OP_MODE) != (MODE_LONG_RANGE_MODE | MODE_RX_SINGLE))
	{
		LoRa_Write(REG_FIFO_ADDR_PTR, 0);
		LoRa_Write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_SINGLE);
	}
	return packetLength;
}

uint8_t LoRa_Init(double freq, uint8_t dbm, uint8_t sw)
{
	HAL_GPIO_WritePin(LoRa_CS_GPIO_Port, LoRa_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LoRa_REST_GPIO_Port, LoRa_REST_Pin, GPIO_PIN_RESET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LoRa_REST_GPIO_Port, LoRa_REST_Pin, GPIO_PIN_SET);
	HAL_Delay(50);
	
	if(0x12 != LoRa_Read(REG_VERSION))
		return 0;
	
	LoRa_Sleep();
	LoRa_SetFreq((uint32_t)freq);
	
	LoRa_Write(REG_FIFO_TX_BASE_ADDR, 0);
	LoRa_Write(REG_FIFO_RX_BASE_ADDR, 0);
	
	LoRa_Write(REG_LNA, LoRa_Read(REG_LNA) | 0x03);
	LoRa_Write(REG_MODEM_CONFIG_3, 0x04);
	
	LoRa_SetTxPower(dbm);
	LoRa_SetSpreadingFactor(11);
	LoRa_SetSignalBandwidth(125E3);
	LoRa_SetSyncWord(sw);
	LoRa_SetCrc(ON);
	LoRa_Stdby();
	return 1;
}
