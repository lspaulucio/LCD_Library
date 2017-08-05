#include "LCD.h"
/**
* @author Leonardo Santos Paulucio
* @file  LCD.c
* @brief Implementations of LCD library methods
* @copyright All Rights Reserved
*/


/*******************************************************************
*   IMPLEMENTATION
*******************************************************************/

/**
 * A constructor
 *
 * This function create a new LCD with the defalts values defined in LCD.h
 */
LCD LCD_create(void)
{
    LCD lcd;
    lcd.ControlPeripheral = LCD_CTRL_PERIPHERAL;
    lcd.ControlPortBase = LCD_CTRL_PORT;
    lcd.en = ENABLE_PIN;
    lcd.rs = RS_PIN;
    lcd.rw = RW_PIN;

    lcd.DataPeripheral = LCD_DATA_PERIPHERAL;
    lcd.DataPortBase = LCD_DATA_PORT;

    unsigned int i;
    for(i = 0; i < 8; i++)
        lcd.data[i] = 0;

    lcd.data[7] = LCD_D7;
    lcd.data[6] = LCD_D6;
    lcd.data[5] = LCD_D5;
    lcd.data[4] = LCD_D4;

    lcd.column = 0;
    lcd.line = 1;

    return lcd;
}

 /**
  * Control bus pins.
  * This function returns all pins that are used to control the LCD.
  * @param lcd Pointer to a lcd structure.
  * @see LCD_create()
  * @return All control pins that are used.
  */
uint8_t LCD_CTRL_BUS(LCD *lcd)
{
    return (lcd->en | lcd->rs | lcd->rw );
}

/**
 * Data bus pins.
 * This function returns all pins that are used to send data to LCD.
 * @param lcd Pointer to a lcd structure.
 * @see LCD_create()
 * @return All data pins that are used.
 */
 uint8_t LCD_DATA_BUS(LCD *lcd)
{
    uint8_t pins = 0;
    unsigned int i;
    for(i = 4; i < 8; i++)  //Using two nibbles to comunication
        pins = pins | lcd->data[i];
    return pins;
}

/**
 * LCD Setup.
 * This function actives all peripherals used by LCD.
 * @param lcd Pointer to a lcd structure.
 * @see LCD_create()
 */

void LCD_setup(LCD *lcd)
{
    //Enable CTRL Peripheral
    SysCtlPeripheralEnable(lcd->ControlPeripheral);
    GPIOPinTypeGPIOOutput(lcd->ControlPortBase, LCD_CTRL_BUS(lcd));

    //Enable DATA Peripheral
    SysCtlPeripheralEnable(lcd->DataPeripheral);
    GPIOPinTypeGPIOOutput(lcd->DataPortBase, LCD_DATA_BUS(lcd));
}


/**
 * LCD Initialize.
 * This function initialize the LCD by sending configurations commands.
 * @param lcd Pointer to a lcd structure.
 * @see LCD_setup()
 * @see LCD_sendNibble()
 */
void LCD_init(LCD *lcd)
{
            LCD_setup(lcd);
            //Coloca pinos de controle em 0
            GPIOPinWrite(LCD_CTRL_PORT, LCD_CTRL_BUS(lcd), 0x00);
            // Garante que os pinos RS, RW e ENABLE  estão em 0 (low)
            //Coloca pinos de dados em 0
            GPIOPinWrite(LCD_DATA_PORT, LCD_DATA_BUS(lcd), 0x00);
            //Garante que o barramento de dados esta em 0 (low)
            SysCtlDelay(DELAY_15ms);            //Tempo para estabilizar LCD

            // Sequencia de comandos para inicializar e configurar LCD
            LCD_sendNibble(lcd, 0x03);          // Envia comando para inicializar o display
            SysCtlDelay(DELAY_5ms);             // Aguarda 5ms para estabilizar o LCD
            LCD_sendNibble(lcd, 0x03);          // Envia comando para inicializar o display
            SysCtlDelay(DELAY_5ms);             // Aguarda 5ms para estabilizar o LCD
            LCD_sendNibble(lcd, 0x03);          // Envia comando para inicializar o display
            SysCtlDelay(DELAY_5ms);             // Aguarda 5ms para estabilizar o LCD
            LCD_sendNibble(lcd, 0x02);          // CURSOR HOME - Envia comando para zerar o contador de caracteres e retornar à posição inicial (0x80).
            SysCtlDelay(DELAY_5ms);             // Aguarda 5ms para estabilizar o LCD
            LCD_sendByte(lcd, 0, FOUR_BIT);     // FUNCTION SET - Configura o LCD para 4 bits e 2 linhas, fonte 5X7.
            LCD_clear(lcd);                     // Limpa o LCD
            LCD_sendByte(lcd, 0, LCD_TURN_ON);  // DISPLAY CONTROL - Display ligado, sem cursor
            LCD_sendByte(lcd, 0, SHIFT_OFF);    // ENTRY MODE SET - Desloca o cursor para a direita
}

/**
 * LCD Clear.
 * This function cleans the LCD display.
 * @param lcd Pointer to a lcd structure.
 * @see LCD_sendNibble()
 */
void LCD_clear(LCD *lcd) //Limpa display
{
    LCD_sendByte(lcd, 0, LCD_CLEAR);
    SysCtlDelay(DELAY_5ms);
}

/**
 * Send Nibble.
 * This function sends a nibble to LCD.
 * @param lcd Pointer to a lcd structure.
 * @param data Nibble that will be transmitted.
 */
void LCD_sendNibble(LCD *lcd, unsigned char data)
{
	int i;
    for (i = 3; i >= 0; i--)
    {
 	   if (data & (1 << i))
       {
        	GPIOPinWrite(lcd->DataPortBase, lcd->data[4+i], lcd->data[4+i]); //Bit equals 1
       }
       else
       {
        	GPIOPinWrite(lcd->DataPortBase, lcd->data[4+i], 0); //Bit equals 0
       }
    }
    SysCtlDelay(DELAY_12us);
    GPIOPinWrite(LCD_CTRL_PORT, lcd->en, lcd->en); //Enable = 1
    SysCtlDelay(DELAY_12us);
    GPIOPinWrite(LCD_CTRL_PORT, lcd->en, 0); //Enable = 0
}

/**
 * Send Byte.
 * This function sends a byte to LCD.
 * @param lcd Pointer to a lcd structure.
 * @param rs The RS value represents if a byte is an instruction(0) or a data(1).
 * @param byte Byte that will be transmitted.
 * @see LCD_sendNibble()
 * @see LCD_inc_cnt()
 */
void LCD_sendByte(LCD *lcd, bool rs, unsigned char byte)
{
    GPIOPinWrite(LCD_CTRL_PORT, lcd->en, 0); //garantia que desativa enable

    if(rs)
    {
        LCD_inc_cnt(lcd);
        GPIOPinWrite(LCD_CTRL_PORT, lcd->rs, lcd->rs); //RS = 1 Data
    }
    else
    {
        GPIOPinWrite(LCD_CTRL_PORT, lcd->rs, 0); //RS = 0 Intruction
    }

    LCD_sendNibble(lcd, byte >> 4); // Envia a parte ALTA do dado/comando
    LCD_sendNibble(lcd, (byte & 0x0f));  // Limpa a parte ALTA e envia a parte BAIXA do dado/comando
}

/**
 * Send String.
 * This function sends a string to LCD.
 * @param lcd Pointer to a lcd structure.
 * @param word The string that will be transmitted to LCD.
 * @see LCD_sendByte()
 */
void LCD_sendString(LCD *lcd, char* word)
{
    unsigned int i, len = strlen(word);

    for(i = 0; i < len; i++)
    {
        LCD_sendByte(lcd, 1, word[i]);
    }
}

/**
 * Counter Increment.
 * This function controls the LCD cursor.
 * @param lcd Pointer to a lcd structure.
 * @see LCD_create()
 */
void LCD_inc_cnt(LCD *lcd)
{
    if(++(lcd->column) > 16)
    {
        lcd->column = 1; //contando o novo character

        if(lcd->line == 1)
        {
            LCD_sendByte(lcd, 0, 0xC0); //pula pra segunda linha
            lcd->line = 2;
        }
        else if(lcd->line == 2)
        {
            LCD_sendByte(lcd, 0, 0x80); //pula pra primeira linha
            lcd->line = 1;
        }
    }
}

/**
 * Set Position.
 * This function changes the actual position of cursor.
 * @param lcd Pointer to a lcd structure.
 * @param line The new line of cursor.
 * @param col The new line of cursor.
 * @see LCD_create()
 * @see LCD_sendByte()
 */
void LCD_setPosition(LCD *lcd, unsigned int line, unsigned int col) //define a posicão de inicio
{
    if(line == 1)
    {
        line = 0x80;     //Se setado linha 1, end incial 0x80
        lcd->line = 1;
    }
    else if(line == 2)
    {
        line = 0xC0;     //Se setado linha 2, end incial 0xc0
        lcd->line = 2;
    }

    lcd->column = col - 1;         //subtrai 1 para corrigir posição

    line = line + lcd->column;     //soma ao end inicial, o numero da coluna


    LCD_sendByte(lcd, 0, (char)line);

    return;
}

/**
 * Send Number.
 * This function sends a number to LCD Display.
 * @param lcd Pointer to a lcd structure.
 * @param num The number you want to send.
 * @see LCD_sendByte()
 */
void LCD_sendNum(LCD *lcd, unsigned int num)
{
    int number = 1000;
    int r, q, i;

    for(i=0;i<4;i++)
    {
        q = num / number;
        r = num % number;
        LCD_sendByte(lcd, 1, (char)(q+48));
        number /= 10;
        num = r;
    }
}
