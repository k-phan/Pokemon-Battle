/* Khai Phan & Jerome Andaya
 * EC450 - Pokemon Project
 */

/*	BOARD VARIABLES */
#define MASTER 0		// Master - 0
#define SLAVE 1			// Slave - 1
#define MS_TRIGGER 0	// trigger for compiling for master or for slave

/*	DISPLAY VARIABLES 	*/
#define MAX_MOVE_STRING 9
#define MAX_TYPE_STRING 3
#define MAX_NAME_STRING 12

/* INCLUDES */
#include <stdlib.h>
#include "msp.h"
#include <driverlib.h>
#include <grlib.h>
#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <stdint.h>
#include "pokemon_prototypes.h"

/* Graphic Library Context */
Graphics_Context g_sContext;

/* ADC14 VARIABLES */
static uint16_t resultsBuffer[2];	// used to store the x/y calculations from Joystick

/* Function Prototypes */
void initDisplay(void);
void initClock(void);
void initInterrupts(void);
void initSpiMaster(void);
void initSpiSlave(void);
void mapPorts(void);
void initButtons(void);
void checkButtons(void);

/* Variables */
static volatile uint8_t rxData, txData;
volatile char state = 0;
uint8_t myPokemon = 0;
static volatile uint8_t receiveArray[2];
int selectScreenDebounce = 0;
unsigned char last_button1;
int transmitCounter = 0;
int selectCounter = 0;
int rectX = 0;
int rectY = 0;
uint8_t myMove;

/* Port Mapping Config Parameter */
const uint8_t portMapping[] =
{
	PM_UCA2CLK, PM_NONE, PM_UCA2SOMI, PM_UCA2SIMO,
	PM_NONE, PM_NONE, PM_NONE, PM_NONE
};

/* ---------------------------------------
 *				 M A I N
 * ---------------------------------------
 */
void main(void)
{
	/* Initialize all that needs to be initialized */
	mapPorts();
	initClock();
	initDisplay();
	initInterrupts();
	initButtons();

//	if(MS_TRIGGER == MASTER)
//	{
//		txData = 0x00;
//		while(!(SPI_getInterruptStatus(EUSCI_A2_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT)));
//		if(MS_TRIGGER == MASTER)
//			SPI_transmitData(EUSCI_A2_BASE, txData);
//
//		/* SET TO 1 SO NEVER CALLED AGAIN */
////		initFirstTransmit = 1;
//	}

	SPI_transmitData(EUSCI_A2_BASE, 0x00);

	/* Put into low power mode */
	while(1)
	{
		MAP_PCM_gotoLPM0();
		__no_operation();
	}
}

void mapPorts(void)
{
	MAP_PMAP_configurePorts((const uint8_t *) portMapping, P3MAP, 1, PMAP_DISABLE_RECONFIGURATION);
}

/* SPI Master Config Parameter */
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
	EUSCI_SPI_CLOCKSOURCE_ACLK,
	3000000,
	15000,
	EUSCI_SPI_MSB_FIRST,
	EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,
	EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_HIGH,
	EUSCI_SPI_3PIN
};

/* SPI Slave Config Parameter */
const eUSCI_SPI_SlaveConfig spiSlaveConfig =
{
	EUSCI_SPI_MSB_FIRST,
	EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,
	EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_HIGH,
	EUSCI_SPI_3PIN
};

/* Initialize the Clock */
void initClock(void)
{
	/* Halting WDT and disabling all interrupts */
	MAP_WDT_A_holdTimer();
	MAP_Interrupt_disableMaster();

	/* Set the core voltage level to VCORE1 */
	MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

	/* Set 2 flash wait states for flash bank 0 & 1 */
	MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
	MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

	/* -- THE ACTUAL CLOCK INITIALIZATION -- */
	MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
	MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

/* Initialize the Display */
void initDisplay(void)
{
	/* Initialize Display */
	Crystalfontz128x128_Init();
	Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

	/* Initialize Graphics Context */
	Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
	Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
	Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
	GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
	Graphics_clearDisplay(&g_sContext);

	/* Draw Title */
	if(state == 0)
		draw_title(&g_sContext, 3, 1);
}

/* Initialize all of the Interrupts */
void initInterrupts(void)
{
	/* Configures Pin 6.0 and 4.4 as ADC input */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0,
		GPIO_TERTIARY_MODULE_FUNCTION);
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4,
		GPIO_TERTIARY_MODULE_FUNCTION);

	/* Initialize ADC */
	MAP_ADC14_enableModule();
	MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

	/* Configuring ADC Memory (ADC_MEM0 - ADC_MEM1 (A15, A9) -- with REPEAT) */
	MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
	MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
		ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);
	MAP_ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS,
		ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

	/* Enabling the Interrupt */
	MAP_ADC14_enableInterrupt(ADC_INT1);

	/* Enabling ALL Interrupts */
	MAP_Interrupt_enableInterrupt(INT_ADC14);
	MAP_Interrupt_enableMaster();

	/* Extra Stuff for ADC14 */
	MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
	MAP_ADC14_enableConversion();
	MAP_ADC14_toggleConversionTrigger();

	/* INITIALIZE MASTER OR SLAVE HERE */
	if(MS_TRIGGER == MASTER)
		initSpiMaster();
	else if(MS_TRIGGER == SLAVE)
		initSpiSlave();
}

void initButtons(void)
{
	/* Initialize Buttons and Last Button States */
    P5->DIR &= ~BIT1;				// Clear Direction
    last_button1 = P5->IN & BIT1;	// Initialize the previous state of the button1
}

void checkButtons(void)
{
	/* Handles Button 1 */
	unsigned char button1;
	button1 = P5->IN & BIT1;
	if((button1 == 0) && last_button1)
	{
		/* WHAT HAPPENS WHEN BUTTON 1 IS PRESSED */

		switch(state)
		{
			case 0:
				state = 3;
				break;
			case 1:
				// waiting state
				myMove = (2*rectY) + (1*rectX);
				state = 5;
				break;
			case 2:
				state = 0;
				Graphics_clearDisplay(&g_sContext);
				draw_title(&g_sContext, 3, 1);
				selectScreenDraw(myPokemon, &g_sContext, 70, 63);
				break;
			default:
			break;
		}
	}
	last_button1 = button1;
}

/* Initialize SPI Master */
void initSpiMaster(void)
{
	/* CLK, MOSI, MISO */
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Configure SPI in 3-Wire Master Mode */
	SPI_initMaster(EUSCI_A2_BASE, &spiMasterConfig);

	/* Enable SPI Module */
	SPI_enableModule(EUSCI_A2_BASE);

	/* Enable Interrupts */
	SPI_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
	Interrupt_enableInterrupt(INT_EUSCIA2);
	Interrupt_enableSleepOnIsrExit();
}

/* Initialize SPI Slave */
void initSpiSlave(void)
{
	/* CLK, MOSI, MISO */
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Configure SPI in 3-Wire Master Mode */
	SPI_initSlave(EUSCI_A2_BASE, &spiSlaveConfig);

	/* Enable SPI Module */
	SPI_enableModule(EUSCI_A2_BASE);

	/* Enable Interrupts */
	SPI_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
	Interrupt_enableInterrupt(INT_EUSCIA2);
	Interrupt_enableSleepOnIsrExit();
}

/* ---------------------------------------
 *		 G A M E 	C O N S T S
 * ---------------------------------------
 */

/*
	KEY
	==========================
	0: 		Immune
	0.5:	Not Very Effective
	1: 		Normal
	2: 		Super Effective


	type_advantage[i][j]
	======================
	i 		Attacker
	j 		Defender

	Index		Type
	======================
	0			NRM
	1			FGT
	2			FLY
	3			PSN
	4			GND
	5			RCK
	6			BUG
	7			GHS
	8			STL
	9			FIR
	10			WTR
	11			GRS
	12			ELC
	13			PSY
	14			ICE
	15			DRG
	16			DRK


*/


/* ---------------------------------------
 *			I N T E R R U P T S
 * ---------------------------------------
 */
int initFirstTransmit = 0;


uint8_t enemyPokemon;
uint8_t myLastPokemon = 10;
int arrow_index = 0;
int case8Counter = 0;
double myHP;
double enHP;
double onYouEff;
double onEneEff;

uint8_t enMove;

//char myName[MAX_NAME_STRING] = "wooo";
//char enName[MAX_NAME_STRING] = "hi noob";

void ADC14_IRQHandler(void)
{
	/* Send first packet as master if you are master */
	if(MS_TRIGGER == MASTER && initFirstTransmit == 0)
	{
		txData = 0x00;
		while(!(SPI_getInterruptStatus(EUSCI_A2_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT)));
		if(MS_TRIGGER == MASTER)
			SPI_transmitData(EUSCI_A2_BASE, txData);

		/* SET TO 1 SO NEVER CALLED AGAIN */
		initFirstTransmit = 1;
	}
	checkButtons();

	//Graphics_clearDisplay(&g_sContext);

	uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
	MAP_ADC14_clearInterruptFlag(status);

	/* Check if ADC_MEM1 Conversion is Completed */
	if(status & ADC_INT1)
	{
		/* Store ADC14 Conversion Results */
		resultsBuffer[0] = ADC14_getResult(ADC_MEM0);
		resultsBuffer[1] = ADC14_getResult(ADC_MEM1);


		switch(state)
		{
			/*	Beg Case 0: Select 	*/
			case 0:
				if (resultsBuffer[1] > 16000 && rectX < 10) rectX++;
				else if (resultsBuffer[1] < 1000 && rectX >= 0) rectX--;

				if(resultsBuffer[0] > 11000)
				{
					if(selectScreenDebounce == 10){
						switch(myPokemon){
						case 9:
							myPokemon = 0;
							break;
						default:
							myPokemon++;
							break;
						}
						selectScreenDebounce = 0;
					}
				}
				else if(resultsBuffer[0] < 5000)
				{
					if(selectScreenDebounce == 10){
						switch(myPokemon){
						case 0:
							myPokemon = 9;
							break;
						default:
							myPokemon--;
							break;
						}
						selectScreenDebounce = 0;
					}
				}

				if(selectScreenDebounce < 10)
					selectScreenDebounce++;
				// title is 120 by 58

				Graphics_Rectangle select = { 3, 60, 125, 116};
				//Graphics_Rectangle p0 = { 70, 63, 120, 113};
				Graphics_Rectangle p0 = { 69, 62, 121, 114};
				Graphics_Rectangle clearBox = {6, 89, 67, 110};
				/* So that you only draw when screen updates */
				if(myPokemon != myLastPokemon)
				{
					//draw_title(&g_sContext, 3, 1);
					selectScreenDraw(myPokemon, &g_sContext, 70, 63);
					Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
					Graphics_fillRectangle(&g_sContext, &clearBox);
				}
				myLastPokemon = myPokemon;

				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
				Graphics_drawString(&g_sContext, "Pokemon", MAX_NAME_STRING, 7, 62, 0);
				Graphics_drawString(&g_sContext, "Select", MAX_NAME_STRING, 12, 70, 0);
				Graphics_drawString(&g_sContext, pokemon_list[myPokemon][0], MAX_NAME_STRING, 7, 90, 0);
				Graphics_drawString(&g_sContext, pokemon_list[myPokemon][1], MAX_TYPE_STRING, 12, 98, 0);
				if(pokemon_list[myPokemon][2] != NULL)
					Graphics_drawString(&g_sContext, pokemon_list[myPokemon][2], MAX_TYPE_STRING, 40, 98, 0);
				Graphics_drawRectangle(&g_sContext, &select);
				Graphics_drawRectangle(&g_sContext, &p0);

				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
					for (arrow_index = 7; arrow_index >= 0; arrow_index--)
					{
						Graphics_drawLineH(&g_sContext, 115, 115+(7-arrow_index), 87+arrow_index);
						Graphics_drawLineH(&g_sContext, 115, 115+(7-arrow_index), 87-arrow_index);
						Graphics_drawLineH(&g_sContext, 73-(7-arrow_index), 73, 87+arrow_index);
						Graphics_drawLineH(&g_sContext, 73-(7-arrow_index), 73, 87-arrow_index);
					}

			break;
		   				/*	End Case 0: Select 	*/

			/*	Beg Case 1: Battle 	*/
			case 1:
				if (resultsBuffer[0] > 11000) rectX = 1;
				else if (resultsBuffer[0] < 5000) rectX = 0;

				if (resultsBuffer[1] > 11000) rectY = 0;
				else if (resultsBuffer[1] < 5000) rectY = 1;

				int move = (2*rectY) + (1*rectX);
				//Rectangle = {xmin, ymin, xmax, ymax}
				Graphics_Rectangle menu  =  { 3, 78, 125, 125};
				Graphics_Rectangle move0 =  { 5, 80, 60, 100};
				Graphics_Rectangle move1 =  { 68, 80, 123, 100};
				Graphics_Rectangle move2 =  { 5, 103, 60, 123};
				Graphics_Rectangle move3 =  { 68, 103, 123, 123};


				Graphics_Rectangle myHPBar	=	{5, 7, 55, 9};
				Graphics_Rectangle enHPBar	=	{70, 7, 120, 9};

				double hp1 = ((myHP/atoi(pokemon_list[myPokemon][3]))*50);
				double hp2 = ((enHP/atoi(pokemon_list[enemyPokemon][3]))*50);
				Graphics_Rectangle myDmgBar =	{5+(hp1), 7, 55, 9};
				Graphics_Rectangle enDmgBar =	{70+(hp2), 7, 120, 9};


				// Draw Menu

				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
					Graphics_drawRectangle(&g_sContext, &menu);
					if (move != 0) Graphics_drawRectangle(&g_sContext, &move0);
					if (move != 1) Graphics_drawRectangle(&g_sContext, &move1);
					if (move != 2) Graphics_drawRectangle(&g_sContext, &move2);
					if (move != 3) Graphics_drawRectangle(&g_sContext, &move3);

					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 0][0], MAX_MOVE_STRING, 7, 82, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 1][0], MAX_MOVE_STRING, 70, 82, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 2][0], MAX_MOVE_STRING, 7, 107, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 3][0], MAX_MOVE_STRING, 70, 107, 0);

					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 0][1], MAX_TYPE_STRING, 42, 92, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 1][1], MAX_TYPE_STRING, 105, 92, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 2][1], MAX_TYPE_STRING, 42, 116, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 3][1], MAX_TYPE_STRING, 105, 116, 0);

					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 0][2], MAX_TYPE_STRING, 9, 92, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 1][2], MAX_TYPE_STRING, 72, 92, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 2][2], MAX_TYPE_STRING, 9, 116, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon * 4 + 3][2], MAX_TYPE_STRING, 72, 116, 0);

					/*Draw Pokemanz Hurr*/
					// MyPokemon = {5,10}
					// EnPokemon = {70, 10}
					/*-------------------*/
					selectScreenDraw(myPokemon, &g_sContext, 5, 10);
					selectScreenDraw(enemyPokemon, &g_sContext, 70, 10);

					/*-------------------*/

				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
					Graphics_fillRectangle(&g_sContext, &myHPBar);
					Graphics_fillRectangle(&g_sContext, &enHPBar);


				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
					if (move == 0)
						Graphics_drawRectangle(&g_sContext, &move0);
					else if (move == 1)
						Graphics_drawRectangle(&g_sContext, &move1);
					else if (move == 2)
						Graphics_drawRectangle(&g_sContext, &move2);
					else if (move == 3)
						Graphics_drawRectangle(&g_sContext, &move3);

					Graphics_fillRectangle(&g_sContext, &myDmgBar);
					Graphics_fillRectangle(&g_sContext, &enDmgBar);

				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
					Graphics_drawString(&g_sContext, pokemon_list[myPokemon][0], MAX_NAME_STRING, 2, 65, 0);
					Graphics_drawString(&g_sContext, pokemon_list[enemyPokemon][0], MAX_NAME_STRING, 67, 65, 0);

			break;
						/*	End Case 1: Battle 	*/

			/*	Beg Case 2: End 	*/
			case 2:
				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
				if(myHP == 0 && enHP == 0){
					Graphics_drawString(&g_sContext, "You Draw!", 9, 39, 48, 0);
				}
				else if(myHP == 0)
				{
					Graphics_drawString(&g_sContext, "You Lose!", 9, 39, 48, 0);
				}
				else if(enHP == 0)
				{
					Graphics_drawString(&g_sContext, "You Win!!", 9, 39, 48, 0);
				}
			break;
						/*	End Case 2: End 	*/

			/* 3 = wait state between menu and battle */
			case 3:
				Graphics_drawString(&g_sContext, "Connecting...", 13, 30, 48, 0);
			break;

			/* 4 = 2nd wait state between menu and battle */
			case 4:
				// do nothing
			break;

			/* 5 = calculate damage based on move and if it kills -> go to state 2 for end, else go back to 1 */
			case 5:
				//calculate damage go to state 1 or 2
			break;
			case 7: /* calculate damage */
					myHP = myHP - calcDamage(atoi(move_list[enemyPokemon*4 + enMove][2]),
										     atoi(pokemon_list[enemyPokemon][4]),
										     atoi(pokemon_list[myPokemon][5]),
											 atoi(move_list[enemyPokemon*4 + enMove][3]),
										     atoi(pokemon_list[myPokemon][6]),
										     atoi(pokemon_list[myPokemon][7]));

					enHP = enHP - calcDamage(atoi(move_list[myPokemon*4 + myMove][2]),
										     atoi(pokemon_list[myPokemon][4]),
										     atoi(pokemon_list[enemyPokemon][5]),
											 atoi(move_list[myPokemon*4 + myMove][3]),
										     atoi(pokemon_list[enemyPokemon][6]),
										     atoi(pokemon_list[enemyPokemon][7]));

					if(myHP <= 0) myHP = 0;
					if(enHP <= 0) enHP = 0;

					state = 8;
			break;
			case 8:
				if(case8Counter == 0)
				{
					onYouEff = calcEff(atoi(move_list[enemyPokemon*4 + enMove][3]),
										  atoi(pokemon_list[myPokemon][6]),
										  atoi(pokemon_list[myPokemon][7]));

					onEneEff = calcEff(atoi(move_list[myPokemon*4 + myMove][3]),
										  atoi(pokemon_list[enemyPokemon][6]),
										  atoi(pokemon_list[enemyPokemon][7]));

					Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
					Graphics_fillRectangle(&g_sContext, &menu);

					Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
					Graphics_drawString(&g_sContext, "Enemy:", 6, 7, 82, 0);
					Graphics_drawString(&g_sContext, "You:", 4, 7, 107, 0);
					Graphics_drawString(&g_sContext, move_list[enemyPokemon*4 + enMove][0], MAX_MOVE_STRING, 55, 82, 0);
					Graphics_drawString(&g_sContext, move_list[myPokemon*4 + myMove][0], MAX_MOVE_STRING, 55, 107, 0);

					if(onYouEff > 1) Graphics_drawString(&g_sContext, "Super Effective!", 16, 20, 92, 0);
					else if(onYouEff < 1) Graphics_drawString(&g_sContext, "Not effective...", 16, 20, 92, 0);

					if(onEneEff > 1) Graphics_drawString(&g_sContext, "Super Effective!", 16, 20, 116, 0);
					else if(onEneEff < 1) Graphics_drawString(&g_sContext, "Not effective...", 16, 20, 116, 0);
				}

				if(case8Counter != 3000)
					case8Counter++;
				else{
					case8Counter = 0;
					if(enHP == 0 || myHP == 0){
						Graphics_clearDisplay(&g_sContext);
						state = 2;
					}
					else
					{
						state = 1;
						Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
						Graphics_fillRectangle(&g_sContext, &menu);
					}
				}
				break;

			/*	Beg Default 		*/

			default:

			break;
						/* 	End Default 		*/
		}
	}
}

void EUSCIA2_IRQHandler(void)
{
	/* Clear Interrupt Flag */
	uint32_t status = SPI_getEnabledInterruptStatus(EUSCI_A2_BASE);
	SPI_clearInterruptFlag(EUSCI_A2_BASE, status);

	if(status & EUSCI_A_SPI_RECEIVE_INTERRUPT)
	{
		while(!(SPI_getInterruptStatus(EUSCI_A2_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT)));

		rxData = SPI_receiveData(EUSCI_A2_BASE);

		switch(state)
		{
		case 0:
			txData = 0x00;
			selectCounter = 0;
			break;
		case 1:
			txData = 0x00;
			selectCounter = 0;
			break;
		case 2:
			txData = 0x00;
			break;

		case 3:
			txData = 0xFF;
			/* Transmit More Data */
			SPI_transmitData(EUSCI_A2_BASE, txData);
			if(rxData == 0xFF)
			{
				state = 4;
			}
			transmitCounter = 0;
			break;
		case 4:
			if(selectCounter < 8 && transmitCounter > 5){
				txData = myPokemon;
				SPI_transmitData(EUSCI_A2_BASE, txData);
				if(selectCounter >= 2 && selectCounter <= 5)
				{
					if(rxData >= 0 && rxData <= 9)
						enemyPokemon = rxData;
				}
				selectCounter++;
			}
			else if(selectCounter >= 8)
			{
				Graphics_clearDisplay(&g_sContext);
				myHP = atoi(pokemon_list[myPokemon][3]);
				enHP = atoi(pokemon_list[enemyPokemon][3]);
				state = 1;
			}
			transmitCounter++;
			break;
		case 5:
			txData = 0xFF;
			/* Transmit More Data */
			SPI_transmitData(EUSCI_A2_BASE, txData);
			if(rxData == 0xFF)
			{
				state = 6;
			}
			transmitCounter = 0;
			break;
		case 6:
			if(selectCounter < 8 && transmitCounter > 5){
				txData = myMove;
				SPI_transmitData(EUSCI_A2_BASE, txData);
				if(selectCounter >= 2 && selectCounter <= 5)
				{
					if(rxData >= 0 && rxData <= 4)
						enMove = rxData;
				}
				selectCounter++;
			}
			else if(selectCounter >= 8)
			{
				state = 7;
			}
			transmitCounter++;
			break;
		case 7:
			txData = 0x00;
			selectCounter = 0;
			break;
		case 8:
			txData = 0x00;
			break;
		default:
			break;
		}

	}
	SPI_transmitData(EUSCI_A2_BASE, txData);


	/* Delay for Processing of Data */
	int i;
	if(MS_TRIGGER == MASTER)
		for(i = 0; i < 60; i++);
}
