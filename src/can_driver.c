#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "can_config.h"
#include "mcp2515_regs.h"
#include "can_driver.h"
#include <stdbool.h>
#include <stdio.h>

static inline void cs_select() {
    gpio_put(MCP2515_CS, 0);
}

static inline void cs_deselect() {
    gpio_put(MCP2515_CS, 1);
}

void can_init(void) {
    spi_init(SPI_PORT, SPI_BAUDRATE);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    gpio_init(MCP2515_CS);
    gpio_set_dir(MCP2515_CS, true);
    gpio_put(MCP2515_CS, 1);

    // Reset MCP2515
    cs_select();
    uint8_t reset = 0xC0;
    spi_write_blocking(SPI_PORT, &reset, 1);
    cs_deselect();

    sleep_ms(100);

    // Enter Config Mode
    cs_select();
    uint8_t config_mode[] = {0x02, REG_CANCTRL, 0x80};
    spi_write_blocking(SPI_PORT, config_mode, sizeof(config_mode));
    cs_deselect();

    // Read back REG_CANCTRL to verify
    cs_select();
    uint8_t read_cmd[] = {0x03, REG_CANCTRL, 0x00};
    uint8_t resp[3] = {0};
    spi_write_read_blocking(SPI_PORT, read_cmd, resp, 3);
    cs_deselect();

    printf("REG_CANCTRL read-back = 0x%02X\n", resp[2]);

    sleep_ms(2);

    // Write CNF registers
    cs_select();
    uint8_t cnf3[] = {0x02, REG_CNF3, 0x02};
    spi_write_blocking(SPI_PORT, cnf3, sizeof(cnf3));
    cs_deselect();

    cs_select();
    uint8_t cnf2[] = {0x02, REG_CNF2, 0x90};
    spi_write_blocking(SPI_PORT, cnf2, sizeof(cnf2));
    cs_deselect();

    cs_select();
    uint8_t cnf1[] = {0x02, REG_CNF1, 0x03};
    spi_write_blocking(SPI_PORT, cnf1, sizeof(cnf1));
    cs_deselect();

    // Set RX buffers to accept all
    cs_select();
    uint8_t rxctrl0[] = {0x02, REG_RXBnCTRL(0), FLAG_RXM1 | FLAG_RXM0};
    spi_write_blocking(SPI_PORT, rxctrl0, sizeof(rxctrl0));
    cs_deselect();

    cs_select();
    uint8_t rxctrl1[] = {0x02, REG_RXBnCTRL(1), FLAG_RXM1 | FLAG_RXM0};
    spi_write_blocking(SPI_PORT, rxctrl1, sizeof(rxctrl1));
    cs_deselect();

    // Enter Normal Mode
    cs_select();
    uint8_t normal_mode[] = {0x02, REG_CANCTRL, 0x00};
    spi_write_blocking(SPI_PORT, normal_mode, sizeof(normal_mode));
    cs_deselect();

    // Read CANSTAT
cs_select();
uint8_t tx[] = {0x03, REG_CANCTRL, 0x00};
uint8_t rx[3] = {0};
spi_write_read_blocking(SPI_PORT, tx, rx, 3);
cs_deselect();

printf("CANSTAT = 0x%02X\n", rx[2]);
}
void can_send(const can_frame_t* frame) {
    printf("entered can_send()\n");
    printf("preparing to execute cs_select()\n");
    cs_select();
    printf("executed cs_select()\n");

    uint8_t buf[14] = {0};
    buf[0] = 0x40; // LOAD TX BUFFER instruction (TXB0)
    buf[1] = (frame->id >> 3) & 0xFF;
    buf[2] = (frame->id & 0x07) << 5;
    buf[3] = 0; // Extended ID = 0
    buf[4] = 0; // Extended ID = 0
    buf[5] = frame->dlc & 0x0F;

    for (int i = 0; i < frame->dlc; i++) {
        buf[6 + i] = frame->data[i];
    }

    printf("preparing to run spi_write_blocking()\n");
    spi_write_blocking(SPI_PORT, buf, 6 + frame->dlc);
    printf("executed spi_write_blocking()\n");
    printf("preparing to cs_deselect()\n");
    cs_deselect();
    printf("executed cs_deselect()\n");

    // Request to send TXB0
    cs_select();
    uint8_t rts = 0x81;
    spi_write_blocking(SPI_PORT, &rts, 1);
    cs_deselect();
    return;
}

bool can_receive(can_frame_t* frame) {
    // Read CANINTF
    cs_select();
    uint8_t cmd[] = {0x03, REG_CANINTF, 0x00};
    uint8_t resp[3] = {0};
    spi_write_read_blocking(SPI_PORT, cmd, resp, 3);
    cs_deselect();

    uint8_t canintf = resp[2];
//    printf("CANINTF register: 0x%02X\n", canintf);

    int rx_buf = -1;
    if (canintf & FLAG_RXnIF(0)) {
        rx_buf = 0;
    } else if (canintf & FLAG_RXnIF(1)) {
        rx_buf = 1;
    } else {
        return false; // No frame pending
    }

    if (rx_buf >= 0) {
        printf("Receiving from RX buffer %d\n", rx_buf);
    }

    uint8_t read_cmd = (rx_buf == 0) ? 0x90 : 0x94;

    cs_select();
    spi_write_blocking(SPI_PORT, &read_cmd, 1);

    uint8_t buf[13] = {0};
    spi_read_blocking(SPI_PORT, 0x00, buf, 13);
    cs_deselect();

    printf("Raw RX buffer data:");
    for (int i = 0; i < 13; i++) {
        printf(" %02X", buf[i]);
    }
    printf("\n");

    frame->id = ((uint32_t)buf[0] << 3) | (buf[1] >> 5);
    frame->dlc = buf[4] & 0x0F;
    for (int i = 0; i < frame->dlc; i++) {
        frame->data[i] = buf[5 + i];
    }

    printf("Received CAN Frame: ID=0x%lX DLC=%d Data:", frame->id, frame->dlc);
    for (int i = 0; i < frame->dlc; i++) {
        printf(" %02X", frame->data[i]);
    }
    printf("\n");

    // Clear interrupt flag for that RX buffer
    cs_select();
    uint8_t clear_cmd[] = {0x05, REG_CANINTF, FLAG_RXnIF(rx_buf)};
    spi_write_blocking(SPI_PORT, clear_cmd, sizeof(clear_cmd));
    cs_deselect();

    return true;
}