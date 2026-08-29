/*
 * Reconstructed firmware for the STM32 Bluetooth Mobile Robot.
 *
 * The original final source file was not preserved.
 * This implementation was reconstructed from:
 *  - the final project presentation,
 *  - preserved development firmware,
 *  - the recorded working robot demo.
 *
 * It reproduces the documented final system architecture and behaviour.
 */

#include "main.h"
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* Peripheral handles                                                         */
/* -------------------------------------------------------------------------- */

ADC_HandleTypeDef hadc;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
UART_HandleTypeDef huart3;


/* -------------------------------------------------------------------------- */
/* Runtime state                                                              */
/* -------------------------------------------------------------------------- */

/* Ultrasonic measurement */
volatile uint8_t measure = 0;
volatile uint8_t got_time = 0;
volatile uint8_t capture_edge = 0;

volatile uint32_t t1 = 0;
volatile uint32_t t2 = 0;
volatile uint32_t echo_time = 0;

float distance = 0.0f;


/* ADC / potentiometer */
volatile uint32_t adc_new_value = 0;
volatile uint8_t sample_ready = 0;

float voltage = 0.0f;


/* Bluetooth */
volatile uint8_t bluetooth_rx_buffer[1];
volatile uint8_t bt_last = 0;
volatile uint8_t bluetooth_data_ready = 0;


/* -------------------------------------------------------------------------- */
/* Function prototypes                                                        */
/* -------------------------------------------------------------------------- */

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART3_UART_Init(void);

static void process_bluetooth_command(uint8_t command);
static void update_proximity_feedback(float distance_cm, float voltage_v);


/* -------------------------------------------------------------------------- */
/* Main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_USART3_UART_Init();


    /*
     * TIM2 CH1:
     * Ultrasonic echo input capture.
     */
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);


    /*
     * TIM4 CH1:
     * Generates the periodic ~300 ms measurement event.
     */
    HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);


    /*
     * TIM3 CH3:
     * Used for buzzer output-compare behaviour.
     */
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_3);


    /*
     * Start interrupt-driven reception of one Bluetooth byte.
     */
    HAL_UART_Receive_IT(
        &huart3,
        (uint8_t *)bluetooth_rx_buffer,
        1
    );


    while (1)
    {
        /* -------------------------------------------------------------- */
        /* Sensor measurement                                             */
        /* -------------------------------------------------------------- */

        if (measure == 1)
        {
            measure = 0;


            /*
             * Wait for the ultrasonic echo pulse to be measured.
             *
             * This blocking wait reproduces the architecture documented
             * in the original final presentation.
             */
            while (got_time == 0)
            {
            }

            got_time = 0;


            /*
             * HC-SR04-style conversion documented in the original code:
             *
             * distance [cm] = echo_time [us] / 58
             */
            distance = ((float)echo_time) / 58.0f;


            /*
             * Wait for the ADC conversion initiated by the measurement
             * timer callback.
             */
            while (sample_ready == 0)
            {
            }

            sample_ready = 0;


            /*
             * 12-bit ADC conversion.
             *
             * The original presentation used 3.0 V as reference.
             */
            voltage = 3.0f * (float)adc_new_value / 4095.0f;


            update_proximity_feedback(distance, voltage);
        }


        /* -------------------------------------------------------------- */
        /* Bluetooth teleoperation                                        */
        /* -------------------------------------------------------------- */

        if (bluetooth_data_ready == 1)
        {
            bluetooth_data_ready = 0;

            uint8_t command = bt_last;

            /*
             * Ignore CR/LF characters sent by some Bluetooth terminals.
             */
            if (command != '\r' && command != '\n')
            {
                /*
                 * Echo command back to the Bluetooth terminal.
                 */
                HAL_UART_Transmit(&huart3, &command, 1, 100);

                process_bluetooth_command(command);
            }
        }
    }
}


/* -------------------------------------------------------------------------- */
/* Bluetooth command processing                                               */
/* -------------------------------------------------------------------------- */

static void process_bluetooth_command(uint8_t command)
{
    switch (command)
    {
        /*
         * 0 = STOP
         * 1 = FORWARD
         * 2 = LEFT
         * 3 = RIGHT
         * 4 = BACKWARD
         */

        case '0':   /* Stop */

            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

            break;


        case '1':   /* Forward */

            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

            break;


        case '4':   /* Backward */

            /*
             * Reconstructed from the documented H-bridge pin assignment.
             */
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

            break;


        case '2':   /* Left */

            /*
             * Reconstructed differential-drive turn:
             * left motor stopped, right motor forward.
             */
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

            break;


        case '3':   /* Right */

            /*
             * Reconstructed differential-drive turn:
             * right motor stopped, left motor forward.
             */
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

            break;


        default:

            /* Unknown Bluetooth command: no action. */

            break;
    }
}


/* -------------------------------------------------------------------------- */
/* Proximity feedback                                                         */
/* -------------------------------------------------------------------------- */

static void update_proximity_feedback(float distance_cm, float voltage_v)
{
    /*
     * The final presentation documents three buzzer behaviours:
     *
     *  - no sound
     *  - intermittent sound
     *  - continuous sound
     *
     * enabled when the potentiometer voltage is >= 2 V.
     */

    if (voltage_v < 2.0f)
    {
        /*
         * Disable buzzer output.
         */
        TIM3->CCMR2 &= ~(0b111 << 4);
        TIM3->CCMR2 |=  (0b100 << 4);

        return;
    }


    if (distance_cm > 20.0f)
    {
        /* No sound */

        TIM3->CCMR2 &= ~(0b111 << 4);
        TIM3->CCMR2 |=  (0b100 << 4);
    }

    else if (distance_cm >= 10.0f)
    {
        /* Intermittent buzzer */

        TIM3->CCMR2 &= ~(0b111 << 4);
        TIM3->CCMR2 |=  (0b011 << 4);
    }

    else
    {
        /*
         * Continuous sound.
         *
         * Reconstructed output-compare mode based on the final
         * presentation. Exact original register sequence was not
         * fully visible in the surviving slide.
         */

        TIM3->CCMR2 &= ~(0b111 << 4);
        TIM3->CCMR2 |=  (0b001 << 4);
    }
}


/* -------------------------------------------------------------------------- */
/* Interrupt callbacks                                                        */
/* -------------------------------------------------------------------------- */

/*
 * Periodic output-compare callbacks.
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    /*
     * TIM4 schedules a sensor/ADC measurement every ~300 ms.
     */
    if (htim->Instance == TIM4)
    {
        TIM4->CCR1 = TIM4->CNT + 300;

        measure = 1;


        /* Start ADC conversion */
        HAL_ADC_Start_IT(&hadc);


        /*
         * Start ultrasonic trigger pulse on PA1.
         */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

        __HAL_TIM_SET_COUNTER(&htim2, 0);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 12);

        HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2);
    }


    /*
     * TIM3 toggles the buzzer every 250 ms in intermittent mode.
     */
    if (htim->Instance == TIM3)
    {
        TIM3->CCR3 = TIM3->CNT + 250;
    }


    /*
     * TIM2 CH2 finishes the ultrasonic trigger pulse.
     */
    if (
        htim->Instance == TIM2 &&
        htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2
    )
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

        HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
    }
}


/*
 * ADC conversion-complete callback.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc_ptr)
{
    adc_new_value = HAL_ADC_GetValue(hadc_ptr);

    sample_ready = 1;

    HAL_ADC_Stop_IT(hadc_ptr);
}


/*
 * Ultrasonic echo input-capture callback.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (
        htim->Instance == TIM2 &&
        htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1
    )
    {
        capture_edge++;

        if (capture_edge == 1)
        {
            /*
             * Rising edge.
             */
            t1 = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1);
        }

        else
        {
            /*
             * Falling edge.
             */
            capture_edge = 0;

            t2 = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1);


            if (t1 > t2)
            {
                /*
                 * Timer overflow between rising and falling edge.
                 */
                echo_time = 65536U - t1 + t2;
            }

            else
            {
                echo_time = t2 - t1;
            }


            got_time = 1;
        }
    }
}


/*
 * Bluetooth byte received.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        bt_last = bluetooth_rx_buffer[0];

        HAL_UART_Receive_IT(
            &huart3,
            (uint8_t *)bluetooth_rx_buffer,
            1
        );

        bluetooth_data_ready = 1;
    }
}