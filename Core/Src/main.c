#include "main.h"
#include <string.h>
#include "scpi-parser/scpi/scpi.h"// Use https://github.com/j123b567/scpi-parser

UART_HandleTypeDef huart2;
char uart_rx_buf[256];
uint8_t uart_rx_pos = 0;

// SCPI Command Handlers
scpi_result_t DIG_PIN_Set(scpi_t *context) {
    const char *pin;
    int32_t state;

    SCPI_CommandParameters(context, &pin, &state);
    // Example: Parse "PA5" -> GPIOA, GPIO_PIN_5
    GPIO_TypeDef *port = GPIOA;
    uint16_t pin_num = GPIO_PIN_5; // Add your pin mapping logic
    HAL_GPIO_WritePin(port, pin_num, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SCPI_RES_OK;
}

// SCPI Interface Configuration
scpi_interface_t scpi_interface = {
    .error = NULL,
    .write = SCPI_Write, // Implement UART transmit
    .read = NULL, // Not needed for simple commands
};

SCPI_context_t scpi_context;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    // Initialize SCPI parser
    SCPI_Init(&scpi_context, scpi_commands, &scpi_interface, scpi_units_def,
              SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4);

    while (1) {
        // UART Receive Handling
        if (HAL_UART_Receive_IT(&huart2, (uint8_t*)uart_rx_buf, 1) == HAL_OK) {
            if (uart_rx_buf[0] == '\n') {
                SCPI_Input(&scpi_context, uart_rx_buf, uart_rx_pos);
                uart_rx_pos = 0;
            } else {
                uart_rx_pos = (uart_rx_pos + 1) % sizeof(uart_rx_buf);
            }
        }
    }
}

// Implement SCPI_Write for responses
size_t SCPI_Write(scpi_t *context, const char *data, size_t len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)data, len, HAL_MAX_DELAY);
    return len;
}
