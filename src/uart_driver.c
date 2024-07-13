#include <string.h>
#include "uart_driver.h"
#include "peripheral_uart.h"

#include "port_gen_os_driver.h"
#include "platform_api.h"


#define GEN_OS          ((const gen_os_driver_t *)platform_get_gen_os_driver())

#ifndef UART_TX_BUFF_SIZE
#define UART_TX_BUFF_SIZE         (1024)  // must be 2^n
#endif
#ifndef UART_RX_BUFF_SIZE
#define UART_RX_BUFF_SIZE         (128)  // must be 2^n
#endif
#define UART_TX_BUFF_SIZE_MASK   (UART_TX_BUFF_SIZE-1)

typedef struct
{
    uint8_t           tx_buffer[UART_TX_BUFF_SIZE];
    uint8_t           rx_buffer[UART_RX_BUFF_SIZE];    
    uint16_t          write_next;
    uint16_t          read_next;
    gen_handle_t      handle;
    gen_handle_t      tx_sem;
    UART_TypeDef      *port;
    f_uart_rx_bytes   rx_bytes_cb;
    void              *user_data;
} uart_driver_ctx_t;

static uart_driver_ctx_t ctx = {0};

void uart_driver_task(void *data)
{
    uart_driver_ctx_t *ctx = (uart_driver_ctx_t *)data;
    for (;;)
    {
wait:
        if (GEN_OS->event_wait(ctx->tx_sem) != 0) continue;

        while (ctx->read_next != ctx->write_next)
        {
            if (!apUART_Check_TXFIFO_FULL(ctx->port))
            {
                UART_SendData(ctx->port, ctx->tx_buffer[ctx->read_next]);
                ctx->read_next = (ctx->read_next + 1) & UART_TX_BUFF_SIZE_MASK;
            }
            else
                goto wait;
        }
    }
}

void driver_trigger_output(uart_driver_ctx_t *ctx)
{
    GEN_OS->event_set(ctx->tx_sem);
}

uint32_t uart_driver_isr(void *user_data)
{
    uint32_t status;
    char rx[32] = {0};
    uint8_t index = 0;
    while(1)
    {
        status = apUART_Get_all_raw_int_stat(ctx.port);
        if (status == 0)
            break;

        ctx.port->IntClear = status;

        // check rx data (Note: maybe time out happened)
       if (status & ((1 << bsUART_RECEIVE_INTENAB) | (1 << bsUART_TIMEOUT_INTENAB)))
       {
            while (apUART_Check_RXFIFO_EMPTY(ctx.port) != 1)
            {
                char c = (char)ctx.port->DataRead;
                rx[index++] = c;
                
            }
            ctx.rx_bytes_cb(&rx[0], index,status& (1 << bsUART_TIMEOUT_INTENAB));
        }

        // tx int
        if (status & (1 << bsUART_TRANSMIT_INTENAB))
            driver_trigger_output(&ctx);
    }
    return 0;
}

void driver_flush_tx()
{
    while (ctx.read_next != ctx.write_next)
    {
        while (!apUART_Check_TXFIFO_FULL(ctx.port))
        {
            UART_SendData(ctx.port, ctx.tx_buffer[ctx.read_next]);
            ctx.read_next = (ctx.read_next + 1) & UART_TX_BUFF_SIZE_MASK;
        }
    }
}

int uart_add_buffer(uart_driver_ctx_t *ctx, const void *tx_buffer, int size, int start)
{
    int remain = UART_TX_BUFF_SIZE - start;
    if (remain >= size)
    {
        memcpy(ctx->tx_buffer + start, tx_buffer, size);
        return start + size;
    }
    else
    {
        memcpy(ctx->tx_buffer + start, tx_buffer, remain);
        size -= remain;
        memcpy(ctx->tx_buffer, (const uint8_t *)tx_buffer + remain, size);
        return size;
    }
}

int driver_get_free_size(void)
{
    int16_t free_size;
    uint8_t use_mutex = !IS_IN_INTERRUPT();
    if (use_mutex)
        GEN_OS->enter_critical();

    free_size = ctx.read_next - ctx.write_next;
    if (free_size <= 0) free_size += UART_TX_BUFF_SIZE;
    if (free_size > 0) free_size--;

    if (use_mutex)
        GEN_OS->leave_critical();
    return free_size;
}

uint32_t driver_append_tx_data(const void *data, int len)
{
    uint16_t next;
    int16_t free_size;
    uint8_t use_mutex = !IS_IN_INTERRUPT();

    if (use_mutex)
        GEN_OS->enter_critical();

    next = ctx.write_next;
    free_size = ctx.read_next - ctx.write_next;
    if (free_size <= 0) free_size += UART_TX_BUFF_SIZE;
    if (free_size > 0) free_size--;

    if (len > free_size)
    {
        if (use_mutex)
            GEN_OS->leave_critical();
        driver_trigger_output(&ctx);
        return 1;
    }

    next = uart_add_buffer(&ctx, data, len, next) & UART_TX_BUFF_SIZE_MASK;

    ctx.write_next = next;

    if (use_mutex)
        GEN_OS->leave_critical();

    driver_trigger_output(&ctx);
    return 0;
}

void uart_driver_init(UART_TypeDef *port, void *user_data, f_uart_rx_bytes rx_bytes_cb)
{
    ctx.tx_sem = GEN_OS->event_create();
    ctx.port = port;
    ctx.rx_bytes_cb = rx_bytes_cb;
    ctx.user_data = user_data;

    GEN_OS->task_create("uart",
                        uart_driver_task,
                        &ctx,
                        512,
                        GEN_TASK_PRIORITY_LOW);
}

#define COMM_UART_PORT      APB_UART1
#define PIN_COMM_RX         GIO_GPIO_22
#define PIN_COMM_TX         GIO_GPIO_21
UART_sStateStruct config;
static void config_comm_uart(uint32_t freq, uint32_t baud)
{    
	
    config.word_length       = UART_WLEN_8_BITS;
    config.parity            = UART_PARITY_NOT_CHECK;
    config.fifo_enable       = 1;
    config.two_stop_bits     = 0;
    config.receive_en        = 1;
    config.transmit_en       = 1;
    config.UART_en           = 1;
    config.cts_en            = 0;
    config.rts_en            = 0;
    config.rxfifo_waterlevel = 16;
    config.txfifo_waterlevel = 7;
    config.ClockFrequency    = freq;
    config.BaudRate          = baud;

    apUART_Initialize(COMM_UART_PORT, &config, (1 << bsUART_TRANSMIT_INTENAB) | (1 << bsUART_RECEIVE_INTENAB) | (1 << bsUART_TIMEOUT_INTENAB));
    platform_printf("comm baud colck:%d,baud:%d\n",freq,baud);
}

void setup_uart1(void)
{
// UART1
	 SYSCTRL_ClearClkGateMulti(0 | (1 << SYSCTRL_ITEM_APB_UART1)
								 | (1 << SYSCTRL_ITEM_APB_PinCtrl));

    config_comm_uart(SYSCTRL_GetClk(SYSCTRL_ITEM_APB_UART1), 115200);

    PINCTRL_SetPadMux(PIN_COMM_RX, IO_SOURCE_GENERAL);
    PINCTRL_SelUartRxdIn(UART_PORT_1, PIN_COMM_RX);
    PINCTRL_SetPadMux(PIN_COMM_TX, IO_SOURCE_UART1_TXD);
	APB_UART1->DmaCon = (0 << 0) | (0 << 1) | (0 << 2);
	
	platform_set_irq_callback(PLATFORM_CB_IRQ_UART1, (f_platform_irq_cb)uart_driver_isr, NULL);

}
