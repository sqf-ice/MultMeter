/*! @file
********************************************************************************
<PRE>
ģ �� ��     : �����ȡд�����MB85RC64��
�� �� ��     : drvFram.c
����ļ�      :
�ļ�ʵ�ֹ��� :
����         : < 612 >
�汾         : 1.0
--------------------------------------------------------------------------------
��ע         : ���Ա�
--------------------------------------------------------------------------------
�޸ļ�¼ :
  �� ��        �汾      �޸���         �޸�����
2018/05/22   1.0    < 612 >        ����

</PRE>
********************************************************************************

  * ��Ȩ����(c) YYYY, <xxxx>, ��������Ȩ��

*******************************************************************************/


/* Includes--------------------------------------------------------------*/
#include "Include.h"

#define SCL_1         PORT_EE_SCL->BSRR = PIN_EE_SCL
#define SCL_0         PORT_EE_SCL->BRR  = PIN_EE_SCL

#define SDA_1         PORT_EE_SDA->BSRR = PIN_EE_SDA
#define SDA_0         PORT_EE_SDA->BRR  = PIN_EE_SDA

#define SCL_read      PORT_EE_SDA->IDR & PIN_EE_SDA
#define SDA_read      PORT_EE_SDA->IDR & PIN_EE_SDA

void MyI2C_GPIO_OUT_Config(void);
void MyI2C_GPIO_IN_Config(void);
void MyI2C_Start(void);
void MyI2C_Stop(void);
u8   MyI2C_SendByte(u8 ByteData);
u8   MyI2C_ReceiveByte(u8 last_char);

//for 72mhz clk means t=12=>1us
void  delay_us(long t)
{
    long i;

    for(i=0;i<t;i++);
}

//����SDA(PB11)�������
void MyI2C_GPIO_Out_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin   =  PIN_EE_SDA;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_EE_SDA, &GPIO_InitStructure);
}

//����SDA(PB11)��������
void  MyI2C_GPIO_IN_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin   =  PIN_EE_SDA;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(PORT_EE_SDA, &GPIO_InitStructure);
}

//ģ����ʼ�ź�
void MyI2C_Start(void)
{
  MyI2C_GPIO_Out_Config();
  SDA_1;
  SCL_1;
  delay_us(50);
  SDA_0;
  delay_us(50);
  SCL_0;
}

//ģ��ֹͣ�ź�
void MyI2C_Stop(void)
{
  MyI2C_GPIO_Out_Config();
  SDA_0;                                       //I2CStop����������SCL�ǵ͵�ƽ�������
  SCL_1;
  delay_us(50);
  SDA_1;
}

//I2C�����ֽ�
u8 MyI2C_SendByte(u8 ByteData)              //I2C�Ļ������ͷ�ʽ���ֽڷ���
{
  u8 ack;
  u8 i;
  MyI2C_GPIO_Out_Config();
  for (i = 0;  i < 8;  i++)                   //������ͨ��SCL����8��ʱ�����彫Ҫ���͵�
  {                                           //����ͨ��SDA����
    SCL_0;
    delay_us(50);
    if (ByteData&0x80)
      SDA_1;
    else
      SDA_0;
    delay_us(30);
    ByteData <<= 1;                           //���ݽ���ʱ��200us
    SCL_1;
    delay_us(50);                            //���ݱ���ʱ��û��Ҫ��
  }

  SCL_0;                                      //�ڵ�9��ʱ��ʱ��ȡ�Ӽ���ATC1024�����͵�
                                              //ackӦ���źţ���ʱSDAӦΪ����״̬��SD//Ϊ����״̬ʱ��SDA�Ǹߵ�ƽ
  delay_us(50);

  MyI2C_GPIO_IN_Config();                       //ע��������SDAΪ����ʱ��һ��Ҫ��ʹSCL

  SCL_1;                                      //�ǵ͵�ƽ��ǧ����ʹSCLΪ�ߵ�ƽ����Ϊ��
  delay_us(50);                              //SCL�Ǹߵ�ƽ������£��п��ܳ���SDA�ӵ�

  ack = SDA_read ;                            //���ߵ����䣨I2CStop״��������ʧ�ܣ�
  SCL_0;

  return (ack);                               //����ack�źţ��͵�ƽ��ʾ��Ӧ�𣬸���û��
}


//I2C�����ֽ�
u8 MyI2C_ReceiveByte(u8 last_char)
{
  u8 data=0, i;
  MyI2C_GPIO_IN_Config();                       //������ͨ��8��ʱ���źţ����մӼ�����������
  for (i = 0;  i < 8;  i++)
  {
    SCL_0;
    delay_us(50);
    SCL_1;
    delay_us(50);
    data <<=1;
    if (SDA_read ) data++;
  }
  SCL_0;
  delay_us(50);

  MyI2C_GPIO_Out_Config();
  if (last_char)                              //�������������һ���ֽ�ʱ���������ڵ�9������
    SDA_1;                                    //���Ӽ�һ���ߵ�ƽ(NACK)�������һ���͵�ƽ(ACK)
  else                                       //��ʾ��������
    SDA_0;

  delay_us(30);
  SCL_1;
  delay_us(50);
  SCL_0;
  return data;
}
///////////////////////////////�ϰ벿��Ϊ����////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// ����Ϊ�����д����

/* Private macro -------------------------------------------------------------*/
#define FRAM_SIZE 2048
#define FRAM_SECTOR_SIZE 32
#define FRAM_R 1
#define FRAM_W 0

// Addr:8λ�豸�� 16λ��ַ��
#define FRAM_MSBADDRESS(Addr) ((Addr & 0xFF00) >> 8)
#define FRAM_LSBADDRESS(Addr) (Addr & 0xFF)
/********************************************************/
// ��ַ����
#define FRAM_STARTADDR      (0x00000000) 
#define FRAM_DATA_SIZE	              32    // ����������

#define FRAM_Index_ADDR      (0x00000020)    // Di  ��ǩ
#define FRAM_INDEX_SIZE	              10    // ���������� ��ǩҳ

#define FRAM_RECORD_ADDR      (0x00000030)      // DI ��¼ֵ��ַ
#define FRAM_RECORD_SIZE	             364    // ����������        0x0000 019C  

#define FRAM_DO_RECORD_ADDR      (0x000001A0)      // DO ��¼ֵ��ַ +364       0x0000 030c

#define FRAM_MaxDem_ADDR      (0x00000310)      // ���ֵ��¼��ַ
#define FRAM_MaxDem_SIZE	             292    // ���ֵ�������� 0x0000 0434




/********************************************************/

u16 FramData_Crc(u8 *pData)
{
	u16 Crc = 0;
	u16 Counter = 0;
	for (Counter = 0; Counter < FRAM_DATA_SIZE-2; Counter++)
	{
		Crc += *(pData+Counter);
	}
    Crc = Crc & 0xff;
	return Crc;
}

u16 FramRecord_Crc(u8 *pData)
{
	u16 Crc = 0;
	u16 Counter = 0;
	for (Counter = 0; Counter < FRAM_RECORD_SIZE-2; Counter++)
	{
		Crc += *(pData+Counter);
	}
    Crc = Crc & 0xff;
	return Crc;
}

u16 FramIndex_Crc(u8 *pData)
{
	u16 Crc = 0;
	u16 Counter = 0;
	for (Counter = 0; Counter < FRAM_INDEX_SIZE-2; Counter++)
	{
		Crc += *(pData+Counter);
	}
    Crc = Crc & 0xff;
	return Crc;
}

u16 FramMax_Crc(u8 *pData)
{
	u16 Crc = 0;
	u16 Counter = 0;
	for (Counter = 0; Counter < FRAM_MaxDem_SIZE-2; Counter++)
	{
		Crc += *(pData+Counter);
	}
    Crc = Crc & 0xff;
	return Crc;
}

u16 FramEnergy_Crc(u8 *pData)
{
	u32 Crc = 0;
	u16 Counter = 0;
	for (Counter = 0; Counter < FRAM_Energy_SIZE-2; Counter++)
	{
		Crc += *(pData+Counter);
	}
    Crc = Crc & 0xff;
	return Crc;
}

/**
  * @brief  д����
  * @param  -uint16_t phyAddr:������ַ
  			-uint8_t *pWriteData: Ҫд������ݻ���
  			-uint16_t Length:���ݳ���
  * @retval : IIC_ERR_NO_ACK-û��Ӧ��
  				IIC_ERR_NONE-��ȷ
  */
u8 FRAM_I2C_WriteData(u16 phyAddr, u8 *pWriteData, u16 Length)      // ���������ֲ�д������ 
{
	u8 Addr;
	u16 NumByteToWrite = Length;

    MyI2C_Start();

	// ����������ַ
	Addr = 0xA0 | FRAM_W;
    if(MyI2C_SendByte(Addr)) // д�豸��
    {
        MyI2C_Stop();
        return IIC_ERR_NO_ACK;
    }

    // ���͸�8λ��ַ
    Addr = FRAM_MSBADDRESS(phyAddr);
    if(MyI2C_SendByte(Addr)) // доƬ�͵�ַ
    {
        MyI2C_Stop();
        return IIC_ERR_NO_ACK;
    }

	// ���͵�8λ��ַ
	Addr = FRAM_LSBADDRESS(phyAddr);
    if(MyI2C_SendByte(Addr)) // доƬ�͵�ַ
    {
        MyI2C_Stop();
        return IIC_ERR_NO_ACK;
    }

	while(NumByteToWrite--)
	{
		if(MyI2C_SendByte(*pWriteData))
			return IIC_ERR_NO_ACK;

		pWriteData++;
	}

    MyI2C_Stop();

    return IIC_ERR_NONE;
}

/**
  * @brief  ������
  * @param  -uint16_t phyAddr:������ַ
  			-uint8_t *pWriteData: Ҫ��ȡ�����ݻ���
  			-uint16_t Length:���ݳ���
  * @retval : IIC_ERR_NO_ACK-û��Ӧ��
  				IIC_ERR_NONE-��ȷ
  */
u8 FRAM_I2C_ReadData(u16 phyAddr, u8 *pReadData, u16 Length)
{
	u8 Addr;
	u16 NumByteToRead = Length;
    u16 i;
    MyI2C_Start();

	// ����������ַ
	Addr = 0xA0 | FRAM_W;
    if(MyI2C_SendByte(Addr)) // д�豸��
    {
        MyI2C_Stop();
        return IIC_ERR_NO_ACK;
    }

    // ���͸�8λ��ַ
    Addr = FRAM_MSBADDRESS(phyAddr);
    if(MyI2C_SendByte(Addr)) // доƬ�͵�ַ
    {
        MyI2C_Stop();
        return IIC_ERR_NO_ACK;
    }

	// ���͵�8λ��ַ
	Addr = FRAM_LSBADDRESS(phyAddr);
    if(MyI2C_SendByte(Addr)) // доƬ�͵�ַ
    {
        MyI2C_Stop();
        return IIC_ERR_NO_ACK;
    }

    MyI2C_Start();

	// ����������ַ
	Addr = 0xA0 | FRAM_R;
    if(MyI2C_SendByte(Addr)) // д�豸��
    {
        MyI2C_Stop();
        return IIC_ERR_NO_ACK;
    }
    
	for (i = 0;  i < NumByteToRead - 1 ;  i++)
      *pReadData++ = MyI2C_ReceiveByte(0);
    *pReadData++ = MyI2C_ReceiveByte(1);
    MyI2C_Stop();
    return IIC_ERR_NONE;
}
void FRAM_WriteData(void)
{
	WRITE_UNPROTECT;
	u8 FramWriteData[FRAM_DATA_SIZE];
	u16 Size = 0;
	u16 CrcSum = 0;
	u8 *pData;

	memset((u8 *)&FramWriteData, 0xFF, FRAM_DATA_SIZE);
	Size = sizeof(Energy_Memory);
	memcpy((u8 *)&FramWriteData, (u8 *)&ElectricEnergy, Size);

	pData = (u8 *)&FramWriteData[0];
	CrcSum= FramData_Crc(pData);
	pData += (FRAM_DATA_SIZE-2);
	memcpy(pData, (u8 *)&CrcSum, 2);
	FRAM_I2C_WriteData(FRAM_STARTADDR, FramWriteData, FRAM_DATA_SIZE);
	WRITE_PROTECT;
}
void FRAM_ReadData(void)
{
    u8 FramReadData[FRAM_DATA_SIZE];
    for(u8 i =0;i<FRAM_DATA_SIZE;i++)
    {
        FramReadData[i] = 0;
    }
	u16 Size = 0;
	u16 Crc = 0;
	__nop();

	FRAM_I2C_ReadData(FRAM_STARTADDR, FramReadData, FRAM_DATA_SIZE);
	Crc = FramData_Crc(FramReadData);
	if (Crc == FLIPW(&FramReadData[FRAM_DATA_SIZE-2]))  //CRC��֤ ��ȡ�Ƿ����
    {
		Size = sizeof(Energy_Memory);
		memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);

	}
	else // ʧ�ܣ��ѵ�ǰ����д��EEPROM
	{
		//Size = sizeof(Energy_Memory);
		//memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);
	}
}


void FRAM_RecordWrite(void)
{
	WRITE_UNPROTECT;
	u8 FramWriteData[FRAM_RECORD_SIZE];
	u16 Size = 0;
	u16 CrcSum = 0;
	u8 *pData;

	memset((u8 *)&FramWriteData, 0xFF, FRAM_RECORD_SIZE);
	Size = sizeof(SOE_DataStruct)*40;
	memcpy((u8 *)&FramWriteData, (u8 *)&DinRecord[0], Size);

	pData = (u8 *)&FramWriteData[0];
	CrcSum= FramRecord_Crc(pData);
	pData += (FRAM_RECORD_SIZE-2);
	memcpy(pData, (u8 *)&CrcSum, 2);
	FRAM_I2C_WriteData(FRAM_RECORD_ADDR, FramWriteData, FRAM_RECORD_SIZE);
	WRITE_PROTECT;
}
void FRAM_RecordRead(void)
{
    u8 FramReadData[FRAM_RECORD_SIZE];
    for(u16 i =0;i<FRAM_RECORD_SIZE;i++)
    {
        FramReadData[i] = 0;
    }
	u16 Size = 0;
	u16 Crc = 0;

	FRAM_I2C_ReadData(FRAM_RECORD_ADDR, FramReadData, FRAM_RECORD_SIZE);
	Crc = FramRecord_Crc(FramReadData);
	if (Crc == FLIPW(&FramReadData[FRAM_RECORD_SIZE-2]))  //CRC��֤ ��ȡ�Ƿ����
    {
		Size = sizeof(SOE_DataStruct)*40;
		memcpy((u8 *)&DinRecord[0], (u8 *)&FramReadData[0], Size);

	}
	else // ʧ�ܣ��ѵ�ǰ����д��EEPROM
	{
		//Size = sizeof(Energy_Memory);
		//memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);
	}
}

//===================================================================
//���кŶ�д
void FRAM_IndexWrite(void)
{
	WRITE_UNPROTECT;
	u8 FramWriteIndex[FRAM_INDEX_SIZE];
	u16 Size = 0;
	u16 CrcSum = 0;
	u8 *pData;

	memset((u8 *)&FramWriteIndex, 0xFF, FRAM_INDEX_SIZE);
	Size = sizeof(SOE_IndexStruct);
	memcpy((u8 *)&FramWriteIndex, (u8 *)&SoeIndex, Size);

	pData = (u8 *)&FramWriteIndex[0];
	CrcSum= FramIndex_Crc(pData);
	pData += (FRAM_INDEX_SIZE-2);
	memcpy(pData, (u8 *)&CrcSum, 2);
	FRAM_I2C_WriteData(FRAM_Index_ADDR, FramWriteIndex, FRAM_INDEX_SIZE);
	WRITE_PROTECT;
}
void FRAM_IndexRead(void)
{
    u8 FramReadIndex[FRAM_INDEX_SIZE];
    for(u8 i =0;i<FRAM_INDEX_SIZE;i++)
    {
        FramReadIndex[i] = 0;
    }
	u16 Size = 0;
	u16 Crc = 0;

	FRAM_I2C_ReadData(FRAM_Index_ADDR, FramReadIndex, FRAM_INDEX_SIZE);
	Crc = FramIndex_Crc(FramReadIndex);
	if (Crc == FLIPW(&FramReadIndex[FRAM_INDEX_SIZE-2]))  //CRC��֤ ��ȡ�Ƿ����
    {
		Size = sizeof(SOE_IndexStruct);
		memcpy((u8 *)&SoeIndex, (u8 *)&FramReadIndex[0], Size);

	}
	else // ʧ�ܣ��ѵ�ǰ����д��EEPROM
	{
		//Size = sizeof(Energy_Memory);
		//memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);
	}
}

//===================================================================
//DO ����д��
void FRAM_DoRecordWrite(void)
{
	WRITE_UNPROTECT;
	u8 FramWriteDo[FRAM_RECORD_SIZE];
	u16 Size = 0;
	u16 CrcSum = 0;
	u8 *pData;

	memset((u8 *)&FramWriteDo, 0xFF, FRAM_RECORD_SIZE);
	Size = sizeof(SOE_DataStruct)*40;
	memcpy((u8 *)&FramWriteDo, (u8 *)&DoutRecord[0], Size);

	pData = (u8 *)&FramWriteDo[0];
	CrcSum= FramRecord_Crc(pData);
	pData += (FRAM_RECORD_SIZE-2);
	memcpy(pData, (u8 *)&CrcSum, 2);
	FRAM_I2C_WriteData(FRAM_DO_RECORD_ADDR, FramWriteDo, FRAM_RECORD_SIZE);
	WRITE_PROTECT;
}
void FRAM_DoRecordRead(void)
{
    u8 FramReadDo[FRAM_RECORD_SIZE];
    for(u16 i =0;i<FRAM_RECORD_SIZE;i++)
    {
        FramReadDo[i] = 0;
    }
	u16 Size = 0;
	u16 Crc = 0;

	FRAM_I2C_ReadData(FRAM_DO_RECORD_ADDR, FramReadDo, FRAM_RECORD_SIZE);
	Crc = FramRecord_Crc(FramReadDo);
	if (Crc == FLIPW(&FramReadDo[FRAM_RECORD_SIZE-2]))  //CRC��֤ ��ȡ�Ƿ����
    {
		Size = sizeof(SOE_DataStruct)*40;
		memcpy((u8 *)&DoutRecord[0], (u8 *)&FramReadDo[0], Size);

	}
	else // ʧ�ܣ��ѵ�ǰ����д��EEPROM
	{
		//Size = sizeof(Energy_Memory);
		//memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);
	}
}

//===================================================================
//Max ��������д��
void FRAM_MaxDemWrite(void)
{
	WRITE_UNPROTECT;
	u8 FramWriteMax[FRAM_MaxDem_SIZE];
	u16 Size = 0;
	u16 CrcSum = 0;
	u8 *pData;

	memset((u8 *)&FramWriteMax, 0xFF, FRAM_MaxDem_SIZE);
	Size = sizeof(DemMaxStructure)*4;                       // ÿ�ܼ�¼һ�� ���� 
	memcpy((u8 *)&FramWriteMax, (u8 *)&vg_DemMax_Val[0], Size);

	pData = (u8 *)&FramWriteMax[0];
	CrcSum= FramMax_Crc(pData);
	pData += (FRAM_MaxDem_SIZE-2);
	memcpy(pData, (u8 *)&CrcSum, 2);
	FRAM_I2C_WriteData(FRAM_MaxDem_ADDR, FramWriteMax, FRAM_MaxDem_SIZE);
	WRITE_PROTECT;
}
void FRAM_MaxDemRead(void)
{
    u8 FramReadMax[FRAM_MaxDem_SIZE];
    for(u16 i =0;i<FRAM_MaxDem_SIZE;i++)
    {
        FramReadMax[i] = 0;
    }
	u16 Size = 0;
	u16 Crc = 0;

	FRAM_I2C_ReadData(FRAM_MaxDem_ADDR, FramReadMax, FRAM_MaxDem_SIZE);
	Crc = FramMax_Crc(FramReadMax);
	if (Crc == FLIPW(&FramReadMax[FRAM_MaxDem_SIZE-2]))  //CRC��֤ ��ȡ�Ƿ����
    {
		Size = sizeof(DemMaxStructure)*4;
		memcpy((u8 *)&vg_DemMax_Val[0], (u8 *)&FramReadMax[0], Size);
	}
	else // ʧ�ܣ��ѵ�ǰ����д��EEPROM
	{
		//Size = sizeof(Energy_Memory);
		//memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);
	}
}

//===================================================================
// ���ܼ�¼д���ȡ
void FRAM_EnergyRecordWrite(void)
{
	WRITE_UNPROTECT;
	u8 FramWriteEnergy[FRAM_Energy_SIZE];
	u16 CrcSum = 0;
	u8 *pData;
	u16 EnergySize = 0;
	u32 TempAddr;
	memset((u8 *)&FramWriteEnergy, 0xFF, FRAM_Energy_SIZE);
	EnergySize = 589;//sizeof(EnergyRecordStructure);
	memcpy((u8 *)&FramWriteEnergy, (u8 *)&NowEnergyRecord[0], EnergySize);

	pData = (u8 *)&FramWriteEnergy[0];
	CrcSum= FramEnergy_Crc(pData);
	pData += (FRAM_Energy_SIZE-2);
	memcpy(pData, (u8 *)&CrcSum, 2);
    switch (SoeIndex.BackMonth)
    {
        case 0x01:
            TempAddr = FRAM_JanuEnergy_sADDR;  
            break;
        case 0x02:
            TempAddr = FRAM_FebrEnergy_sADDR;  
            break;
        case 0x03:
            TempAddr = FRAM_MarcEnergy_sADDR;  
            break;
        case 0x04:
            TempAddr = FRAM_ApriEnergy_sADDR;  
            break;
        case 0x05:
            TempAddr = FRAM_MayyEnergy_sADDR;  
            break;
        case 0x06:
            TempAddr = FRAM_JuneEnergy_sADDR;  
            break;
        case 0x07:
            TempAddr = FRAM_JulyEnergy_sADDR;  
            break;
        case 0x08:
            TempAddr = FRAM_AuguEnergy_sADDR;  
            break;
        case 0x09:
            TempAddr = FRAM_SeptEnergy_sADDR;  
            break;
        case 0x10:
            TempAddr = FRAM_OctoEnergy_sADDR;  
            break;
        case 0x11:
            TempAddr = FRAM_NoveEnergy_sADDR;  
            break;
        case 0x12:
            TempAddr = FRAM_DeceEnergy_sADDR;  
            break;
        default:
            break;
    }
    FRAM_I2C_WriteData(TempAddr, FramWriteEnergy, FRAM_Energy_SIZE);
	WRITE_PROTECT;
}
void FRAM_EnergyRecordRead(void)
{
    u8 FramReadEnergy[FRAM_Energy_SIZE];
    for(u16 i =0;i<FRAM_Energy_SIZE;i++)
    {
        FramReadEnergy[i] = 0;
    }
	u16 Size = 0;
	u16 Crc = 0;
	
	switch (SoeIndex.BackMonth)
    {
        case 0x01:
            FRAM_I2C_ReadData(FRAM_JanuEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);
            break;
        case 0x02:
            FRAM_I2C_ReadData(FRAM_FebrEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);
            break;
        case 0x03:
            FRAM_I2C_ReadData(FRAM_MarcEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x04:
            FRAM_I2C_ReadData(FRAM_ApriEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x05:
            FRAM_I2C_ReadData(FRAM_MayyEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x06:
            FRAM_I2C_ReadData(FRAM_JuneEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x07:
            FRAM_I2C_ReadData(FRAM_JulyEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x08:
            FRAM_I2C_ReadData(FRAM_AuguEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x09:
            FRAM_I2C_ReadData(FRAM_SeptEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x10:
            FRAM_I2C_ReadData(FRAM_OctoEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x11:
            FRAM_I2C_ReadData(FRAM_NoveEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x12:
            FRAM_I2C_ReadData(FRAM_DeceEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        default:
            break;
    }
	Crc = FramEnergy_Crc(FramReadEnergy);
	if (Crc == FLIPW(&FramReadEnergy[FRAM_Energy_SIZE-2]))  //CRC��֤ ��ȡ�Ƿ����
    {
		Size = 589;//sizeof(EnergyRecordStructure)*31;
		memcpy((u8 *)&NowEnergyRecord[0], (u8 *)&FramReadEnergy[0], Size);

	}
	else // ʧ�ܣ��ѵ�ǰ����д��EEPROM
	{
		//Size = sizeof(Energy_Memory);
		//memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);
	}
}

void FRAM_EnergyRecordSeek(u8 RefKind,u8 RefMonth)
{
    u8 FramReadEnergy[FRAM_Energy_SIZE];
    for(u16 i =0;i<FRAM_Energy_SIZE;i++)
    {
        FramReadEnergy[i] = 0;
    }
	u16 Size = 0;
	u16 Crc = 0;
	
	switch (RefMonth)
    {
        case 0x01:
            FRAM_I2C_ReadData(FRAM_JanuEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);
            break;
        case 0x02:
            FRAM_I2C_ReadData(FRAM_FebrEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);
            break;
        case 0x03:
            FRAM_I2C_ReadData(FRAM_MarcEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x04:
            FRAM_I2C_ReadData(FRAM_ApriEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x05:
            FRAM_I2C_ReadData(FRAM_MayyEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x06:
            FRAM_I2C_ReadData(FRAM_JuneEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x07:
            FRAM_I2C_ReadData(FRAM_JulyEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x08:
            FRAM_I2C_ReadData(FRAM_AuguEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x09:
            FRAM_I2C_ReadData(FRAM_SeptEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x10:
            FRAM_I2C_ReadData(FRAM_OctoEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x11:
            FRAM_I2C_ReadData(FRAM_NoveEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        case 0x12:
            FRAM_I2C_ReadData(FRAM_DeceEnergy_sADDR, FramReadEnergy, FRAM_Energy_SIZE);  
            break;
        default:
            break;
    }
	Crc = FramEnergy_Crc(FramReadEnergy);
	if (Crc == FLIPW(&FramReadEnergy[FRAM_Energy_SIZE-2]))  //CRC��֤ ��ȡ�Ƿ����
    {
		Size = 589; //sizeof(EnergyRecordStructure)*31;
		if(RefKind == FRONTMONTH)
		{
            memcpy((u8 *)&FrontEnergyRecord[0], (u8 *)&FramReadEnergy[0], Size);
		}
		else if(RefKind == REARMONTH)
		{
            memcpy((u8 *)&RearEnergyRecord[0], (u8 *)&FramReadEnergy[0], Size);
		}
	}
	else // ʧ�ܣ��ѵ�ǰ����д��EEPROM
	{
		//Size = sizeof(Energy_Memory);
		//memcpy((u8 *)&ElectricEnergy, (u8 *)&FramReadData[0], Size);
	}
}

/*
void FRAM_Erase_Chip(void)
{
	uint16_t i = 0;
	uint8_t Data[FRAM_SECTOR_SIZE];

	memset(Data, 0xFF, FRAM_SECTOR_SIZE);
	for (i = 0; i < FRAM_SIZE; i+=FRAM_SECTOR_SIZE)
	{
		FRAM_WriteData(i, Data, FRAM_SECTOR_SIZE);
	}
}
// uint16_t Addr ��ʼ��ַ
// uint16_t Num ɾ����sector����
void FRAM_Erase_Sector(uint16_t Addr, uint16_t Num)
{
	uint16_t i = 0;
	uint8_t Data[FRAM_SECTOR_SIZE];
	uint16_t startAddr;
	uint16_t Sector = Addr/FRAM_SECTOR_SIZE;

	if (Addr >= FRAM_SIZE)
	{
		return;
	}

	if ((Addr+Num*FRAM_SECTOR_SIZE) >= FRAM_SIZE)
	{
		return;
	}

	startAddr = Sector * FRAM_SECTOR_SIZE;
	memset(Data, 0xFF, FRAM_SECTOR_SIZE);

	for (i = 0; i < Num; i++)
	{
		FRAM_WriteData(startAddr+i*FRAM_SECTOR_SIZE, Data, FRAM_SECTOR_SIZE);
	}
}*/

// Fram �ϵ�ָ�����
void FramInit(void)
{
    FRAM_ReadData();
    FRAM_RecordRead();
    FRAM_DoRecordRead();
    FRAM_IndexRead();
    FRAM_MaxDemRead();
    FRAM_EnergyRecordRead();
}
