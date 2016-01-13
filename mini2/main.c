
#include "lpc_types.h"

#include "main.h"
#include "led.h"
#include "serialIO.h"
#include "sysTick.h"
#include "i2cIO.h"

void stage1()
{
	listAllDevices();
}

void stage2()
{
	lcdClearScreen();
	lcdPrint("Hello World!\n!dlroW olleH");
}

void stage3()
{
	registerKeyboardInterrupt();
}

int main()
{   
    setupLEDs();
    i2c_init();
	serial_init();
	lcd_init();
	
    //stage1();
    //stage2();
    stage3();
    
    return 0;
}
