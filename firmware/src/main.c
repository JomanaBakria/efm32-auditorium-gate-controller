#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_eusart.h"
#include <stdio.h>
#include "sl_segmentlcd.h"

void init_delay_timer(void);
void delay_ms_timer(uint32_t ms);
void change_PWM(int jump);
void bluetooth(void);
void sendBluetoothMessage(char strring[]);
void check_button_toggle(void);
void check_input(void);
void pwmSet(int dc);
void pwm_init(void);
void set_motor_direction(int direction);

#define PORTD 3
#define OUTPUTPIN 11
#define PWM_FREQ 10000

#define MOTOR_IN1_PORT gpioPortD
#define MOTOR_IN1_PIN  6
#define MOTOR_IN2_PORT gpioPortD
#define MOTOR_IN2_PIN  9
#define BUTTON0_PORT 1
#define BUTTON0_PIN 1
#define BUTTON1_PORT 1
#define BUTTON1_PIN 6

uint32_t topValue;

void app_init(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);//Enable GPIO Clock
  CMU_ClockEnable(cmuClock_TIMER0, true);//Enable Timer0 Clock

  GPIO_PinModeSet(MOTOR_IN1_PORT, MOTOR_IN1_PIN, gpioModePushPull, 0);//Configure PD6 as Output
  GPIO_PinModeSet(MOTOR_IN2_PORT, MOTOR_IN2_PIN, gpioModePushPull, 0);//Configure PD9 as Output
  GPIO_PinModeSet(gpioPortD, 11, gpioModePushPull, 0);//Configure PD11 as Output(the PWM signal pin)

  pwm_init();//Function to Initialize Timer0 as PWM Mode

  GPIO_PinModeSet(BUTTON0_PORT,1,gpioModeInput,0);//Configure Port B pin 1 as an input ( Button0)
  GPIO_PinModeSet(BUTTON1_PORT,6,gpioModeInput,0);//Configure Port B pin 6 as an input ( Button1)

  CMU_ClockEnable(cmuClock_LCD, true);//Enable LCD Clock

  sl_segment_lcd_init(false);//Initialize the LCD Without Turning It ON
   GPIO_PinModeSet(gpioPortD, 13, gpioModeInput, 0);//Configure PD13 as Input (the entry sensor)
   GPIO_PinModeSet(gpioPortD, 10, gpioModeInput, 0);//Configure PD10 as Input (the exist sensor)
   init_delay_timer();//Function to Initialize Timer1 to create delay
  bluetooth();// //Function to Initialize EUSART1
}
void bluetooth(void)
{
  CMU_ClockEnable(cmuClock_EUSART1, true);//Enable EUSART1 Clock

  GPIO_PinModeSet(gpioPortD, 14, gpioModePushPull, 0); // Configure PD14 as Output (Tx-transmitter)
  GPIO_PinModeSet(gpioPortD, 15, gpioModeInput, 0);    // Configure PD15 as Output (Rx-Receiver)

  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;//Initialize the init struct
  init.baudrate = 9600;//Change the default baudrate to 9600 (bit/sec)

  GPIO->EUSARTROUTE[1].TXROUTE =
      (gpioPortD << _GPIO_EUSART_TXROUTE_PORT_SHIFT) |
      (14 << _GPIO_EUSART_TXROUTE_PIN_SHIFT);//Routing the EUSART1 TX to pin PD14
  GPIO->EUSARTROUTE[1].RXROUTE =
      (gpioPortD << _GPIO_EUSART_RXROUTE_PORT_SHIFT) |
      (15 << _GPIO_EUSART_RXROUTE_PIN_SHIFT);//Routing the EUSART1 RX to pin PD15
  GPIO->EUSARTROUTE[1].ROUTEEN =
      GPIO_EUSART_ROUTEEN_RXPEN | GPIO_EUSART_ROUTEEN_TXPEN;//Enable the EUSART1 TX & RX Pins

  EUSART_UartInitHf(EUSART1, &init);//Apply the structure init into EUSART1
}

void pwm_init(void)
{
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;//Initialize the timerInit struct
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;//Initialize the timerCCInit  struct (a compare capture channel )
  timerCCInit.mode = timerCCModePWM;//Change the timerCCInit mode to PWM

  TIMER_Init(TIMER0, &timerInit);//Apply the structure timerInit into TIMER0
  TIMER_InitCC(TIMER0, 0, &timerCCInit);//Apply the structure timerCCInit into TIMER0 CC0

  GPIO->TIMERROUTE[0].ROUTEEN = GPIO_TIMER_ROUTEEN_CC0PEN;//Route CC0 TIMER0 to GPIO pin
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (PORTD << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT) |
      (OUTPUTPIN << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);//Routing the CC0 TIMER0  pin PD11

  uint32_t timerFreq = CMU_ClockFreqGet(cmuClock_TIMER0) / (timerInit.prescale + 1);//39M[HZ]/(0+1)
  topValue = timerFreq / PWM_FREQ;//Calculate how many ticks it takes to the timer0 to reach the signal period
  TIMER_TopSet(TIMER0, topValue);//Set the top timer0 to topValue

  TIMER_Enable(TIMER0, true);//Enable TIMER0
}
void pwmSet(int dc)
{
  uint32_t dutyCount = (topValue * dc) / 100;//Calculate the high Ticks
  TIMER_CompareSet(TIMER0, 0, dutyCount);//Compare between the high ticks and total ticks
}
void sendBluetoothMessage(char string[])
{

      for(int i=0;string[i]!=0;i++){
          EUSART1->TXDATA = string[i];//Transmit the chosen data
      }

}


void set_motor_direction(int direction)
{
  if (direction == 1)//Activate the motor CW
    {
    GPIO_PinOutSet(MOTOR_IN1_PORT, MOTOR_IN1_PIN);
    GPIO_PinOutClear(MOTOR_IN2_PORT, MOTOR_IN2_PIN);
  }
  else if (direction == 0)//Stop the motor
    {


    GPIO_PinOutClear(MOTOR_IN1_PORT, MOTOR_IN1_PIN);
        GPIO_PinOutClear(MOTOR_IN2_PORT, MOTOR_IN2_PIN);
  }
  else//Activate the motor CCW
    {
      GPIO_PinOutSet(MOTOR_IN2_PORT, MOTOR_IN2_PIN);
      GPIO_PinOutClear(MOTOR_IN1_PORT, MOTOR_IN1_PIN);

  }
}
void init_delay_timer(void)
{
    CMU_ClockEnable(cmuClock_TIMER1, true);//Enable TIMER1 Clock

    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;//Initialize the timerInit struct
    timerInit.enable = false; //Do not activate the timer now
    timerInit.prescale = timerPrescale1024;

    TIMER_Init(TIMER1, &timerInit);//Apply the timerInit structure into TIMER1
}
void delay_ms_timer(uint32_t ms)
{

      uint32_t timer_freq = CMU_ClockFreqGet(cmuClock_TIMER1) / 1024;// 39[MHz]/1024=38.085[KHz]
      uint32_t ticks = (timer_freq * ms) / 1000;//Calculate the required ticks

      TIMER_CounterSet(TIMER1, 0);  //Clear the TIMER1
      TIMER_Enable(TIMER1, true);    //Enable TIMER1

      while (TIMER_CounterGet(TIMER1) < ticks); //Wait to TIMER1 to get to the tick
      TIMER_Enable(TIMER1, false);  //Turn Off TIMER1
      }


void app_process_action(void)
{

 check_input();
 delay_ms_timer(100);


 check_button_toggle();
  delay_ms_timer(100);

}


bool motor_forward_on = false;// Tracks whether forward motor is active
bool motor_backward_on = false;// Tracks whether backward motor is active
int count =0;//how many people are in the Auditorium

void check_button_toggle(void) // The check_button_toggle function checks if either of the two buttons was just pressed,
//and based on that toggles the motor's direction (forward or backward),
//updates a step counter, sends a message via Bluetooth,
//updates the LCD, and adjusts the motor speed accordingly.


{
  static bool last_button0 = false;// Stores previous state of button 0
  static bool last_button1 = false;// Stores previous state of button 1
  // Read current button states
  bool current_button0 = (GPIO_PinInGet(BUTTON0_PORT, BUTTON0_PIN) == 0);
  bool current_button1 = (GPIO_PinInGet(BUTTON1_PORT, BUTTON1_PIN) == 0);
  // Check if BUTTON0 was pressed (rising edge detection)
  if (current_button0 && !last_button0)
    {
    motor_forward_on = !motor_forward_on;// Toggle forward motor state

    if (motor_forward_on&& count<5)//Checks if there is available seats
      {
       count++;//Someone entered to the Auditorium
       // Update LCD display with remaining seats
       char buffer[20];
            sprintf(buffer, "E.S=%d\n",5- count);
             sl_segment_lcd_write(buffer);
      motor_backward_on = false;// Ensure backward motor is off
      set_motor_direction(1);// Set direction to CW
      change_PWM(10);// Gradually reduce speed with jumps of 10

    } else {
      set_motor_direction(0);//Stop motor
      pwmSet(0);
    }
  }
  // Check if BUTTON1 was pressed (rising edge detection)
  if (current_button1 && !last_button1)
    {
    motor_backward_on = !motor_backward_on;// Toggle Backrward motor state

    if (motor_backward_on)
      {

       count--;//Someone left to the Auditorium
       // Update LCD display with remaining seats
       char buffer[20];
               sprintf(buffer, "E.S=%d\n",5- count);
                sl_segment_lcd_write(buffer);
      motor_forward_on = false;  // מבטל כיוון קדמי אם היה פעיל
      set_motor_direction(-1);// Set direction to CCW
      change_PWM(10);


    } else {
      set_motor_direction(0);//Stop motor
      pwmSet(0);
    }
  }
  // Save current button states for edge detection
  last_button0 = current_button0;
  last_button1 = current_button1;
}

void check_input(void)// Monitors entrance and exit sensors to control motor direction, update the LCD, and adjust the count.
// If no sensor is triggered, it checks for manual button input and handles motor movement accordingly.
{
  bool adc1_input = (GPIO_PinInGet(gpioPortD, 13) == 1);// Read the state of sensor 1 (e.g., entrance sensor)
  bool adc2_input = (GPIO_PinInGet(gpioPortD, 10) == 1);// Read the state of sensor 2 (e.g., exit sensor)

  if(adc1_input && count <5)// Checks if entrance sensor is triggered and there is available seats
    {
      set_motor_direction(1);
      pwmSet(100);
      count++;//Someone entered to the Auditorium
      // Update LCD with available spots
      char buffer[20];
      sprintf(buffer, "E.S=%d\n",5- count);
      sl_segment_lcd_write(buffer);
      change_PWM(10);// Gradually reduce motor speed with jumps of 10
      pwmSet(0);//Stop Motor
      delay_ms_timer(100);

  }

  else if(adc2_input)
    {
      set_motor_direction(-1);
      pwmSet(100);
      count --;
      char buffer[20];
      sprintf(buffer, "E.S=%d\n", 5 - count);
      sl_segment_lcd_write(buffer);
      change_PWM(10);
        pwmSet(0);
        delay_ms_timer(100);

    }
  else// If no sensor is active, check if a buttons was pressed
    {
      bool current_button0 = (GPIO_PinInGet(BUTTON0_PORT, BUTTON0_PIN) == 0);
      bool current_button1 = (GPIO_PinInGet(BUTTON1_PORT, BUTTON1_PIN) == 0);
      if (!adc1_input && !adc2_input)
        {
          if (current_button1 || current_button0) {
              // Handle button-based motor control
              check_button_toggle();
              // Wait until button is released to avoid repeated trigger
              while (GPIO_PinInGet(BUTTON0_PORT, BUTTON0_PIN) == 0 ||
                     GPIO_PinInGet(BUTTON1_PORT, BUTTON1_PIN) == 0) {
                  delay_ms_timer(10);

              }
          }
      }
  }
}
void change_PWM(int jump)// Gradually reduce PWM duty cycle from 100% to 55% with a chosen jump
{
  for(int dutyCycle=100;dutyCycle>55;dutyCycle-=jump){
             pwmSet(dutyCycle);// Apply new duty cycle
             delay_ms_timer(100);
             // Send current PWM level over Bluetooth
             char buffer[20];
             sprintf(buffer, "PWM=%d\n", dutyCycle);
             sendBluetoothMessage(buffer);
             delay_ms_timer(500);

}
}
