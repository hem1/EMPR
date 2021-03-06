// Serial code
#include "lpc17xx_uart.h"		// Central include files
#include "lpc17xx_pinsel.h"
#include "lpc_types.h"
#include "serialIO.h"			// Local functions


// Read options
int read_usb_serial_none_blocking(char *buf,int length)
{
	return(UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)buf, length, NONE_BLOCKING));
}

// Write options
int write_usb_serial_blocking(char *buf,int length)
{
	return(UART_Send((LPC_UART_TypeDef *)LPC_UART0,(uint8_t *)buf,length, BLOCKING));
}

int stringLength(char targetStr[])
{
    int i = 0;
    unsigned short character = targetStr[0];
    while (character != '\x00')
    {
        i++;
        character = targetStr[i];
    }
    return i;
}

//Useage: myPrintf("[String Content]%[String Content]", int)
//Only support one integer
void printWithInt(char outStr[], int value)
{
    int length = stringLength(outStr);
    
    int hasInteger = 0;
    int i;
    for (i = 0; i<length; i++)
    {
        if (outStr[i] == '%')
        {
            hasInteger = 1;
            break;
        }
    }
    
    if (hasInteger == 1)
    {
        length += sizeof(int)*2 + 1;
        
        char newOutStr[length];
        int j;
        for (j = 0; j<length; j++)
        {
            if (j < i)
            {
                newOutStr[j] = outStr[j];
            }
            else if (j == i)
            {
                //Add 0x prefix
                newOutStr[j] = '0';
                newOutStr[j+1] = 'x';
                j += 2;
                
                //Convert hex into char, and convert little endian into big endian
                int k;
                
                for (k = sizeof(int); k < sizeof(int)*2; k++)
                {
                    unsigned short addition = 0;
                    if (((value >> ((sizeof(int)-k-1)*4)) & 0xf) > 0x9)
                    {
                        addition = 55;
                    }
                    else
                    {
                        addition = 48;
                    }

                    newOutStr[j+k-sizeof(int)] = ((value >> ((sizeof(int)*3-k-1)*4)) & 0xf) + addition;
                }
                
                for (k = 0; k < sizeof(int)*2; k++)
                {
                    unsigned short addition = 0;
                    if (((value >> ((sizeof(int)-k-1)*4)) & 0xf) > 0x9)
                    {
                        addition = 55;
                    }
                    else
                    {
                        addition = 48;
                    }
                    
                    newOutStr[j+k+sizeof(int)] = ((value >> ((sizeof(int)-k-1)*4)) & 0xf) + addition;
                }
                j += sizeof(int)*2 - 1;
            }
            else
            {
                newOutStr[j] = outStr[j-sizeof(int)*2-1];
            }
        }
        write_usb_serial_blocking(newOutStr, length);
    }
    else
    {
        write_usb_serial_blocking(outStr, length);
    }
}

// init code for the USB serial line
void serial_init(void)
{
	UART_CFG_Type UARTConfigStruct;			// UART Configuration structure variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;	// UART FIFO configuration Struct variable
	PINSEL_CFG_Type PinCfg;				// Pin configuration for UART
	/*
	 * Initialize UART pin connect
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	// USB serial first
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);
		
	/* Initialize UART Configuration parameter structure to default state:
	 * - Baudrate = 9600bps
	 * - 8 data bit
	 * - 1 Stop bit
	 * - None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);
	/* Initialize FIFOConfigStruct to default state:
	 * - FIFO_DMAMode = DISABLE
	 * - FIFO_Level = UART_FIFO_TRGLEV0
	 * - FIFO_ResetRxBuf = ENABLE
	 * - FIFO_ResetTxBuf = ENABLE
	 * - FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	// Built the basic structures, lets start the devices/
	// USB serial
	UART_Init((LPC_UART_TypeDef *)LPC_UART0, &UARTConfigStruct);		// Initialize UART0 peripheral with given to corresponding parameter
	UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART0, &UARTFIFOConfigStruct);	// Initialize FIFO for UART0 peripheral
	UART_TxCmd((LPC_UART_TypeDef *)LPC_UART0, ENABLE);			// Enable UART Transmit
	
}
