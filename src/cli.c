#include "pico/stdlib.h"
#include "cli.h"
#include "can_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int read_int_with_echo(const char *prompt);

int read_int_with_echo(const char *prompt) {
    char buffer[16] = {0};
    int idx = 0;
    printf("%s", prompt);
    while (1) {
        int c = getchar();
        if (c == '\r' || c == '\n') {
            printf("\n");
            buffer[idx] = '\0';
            break;
        }
        if (idx < (sizeof(buffer)-1) && c >= 32 && c <= 126) {
            buffer[idx++] = (char)c;
            putchar(c);
        }
    }
    return atoi(buffer);
}

// Default parameters for spaceCAN frame - SX1280
uint8_t default_power_dbm_sx1280 = 10;
uint8_t default_modulation_reg_sx1280 = 0x01;  // LoRa
uint8_t default_bw_reg_sx1280 = 0x26;          // 400 kHz
uint8_t default_sf_reg_sx1280 = 0x70;          // SF7
uint8_t default_cr_reg_sx1280 = 0x01;          // 4/5

// Default parameters for spaceCAN frame - RFM9x
uint8_t default_power_dbm_rfm9x = 10;
uint8_t default_modulation_reg_rfm9x = 0x01;  // LoRa
uint8_t default_bw_reg_rfm9x = 0x07;          // 125 kHz
uint8_t default_sf_reg_rfm9x = 0x70;          // SF7
uint8_t default_cr_reg_rfm9x = 0x01;          // 4/5
uint16_t default_freq_mhz_rfm9x = 915;        // 915 MHz

void cli_task(void *params)
{
    char input[64];

    printf("CANBus Master CLI Ready.\n");

    while (1)
    {
        printf("\nMenu:\n");
        printf("1. Send sx1280 transmission via spaceCAN\n");
        printf("2. Set sx1280 transmission parameters\n");
        printf("3. Send rfm9x transmission via spaceCAN\n");
        printf("4. Set rfm9x transmission parameters\n");
        printf("5. Enable Receive Mode\n");
        printf("Select option: ");
        int c = getchar();
        printf("got char %d\n", c);
        int ch;
        while ((ch = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT && ch != '\n') {}

        can_frame_t frame;
        memset(&frame, 0, sizeof(frame));

        if (c == '1' || c == '3')
        {
            printf("Sending SPACECAN RUN_TEST frame (%s radio)...\n", (c == '1') ? "SX1280" : "RFM9x");

            uint16_t freq_mhz = (c == '1') ? 2250 : default_freq_mhz_rfm9x;
            uint8_t power = (c == '1') ? default_power_dbm_sx1280 : default_power_dbm_rfm9x;
            uint8_t mod = (c == '1') ? default_modulation_reg_sx1280 : default_modulation_reg_rfm9x;
            uint8_t bw = (c == '1') ? default_bw_reg_sx1280 : default_bw_reg_rfm9x;
            uint8_t sf = (c == '1') ? default_sf_reg_sx1280 : default_sf_reg_rfm9x;
            uint8_t cr = (c == '1') ? default_cr_reg_sx1280 : default_cr_reg_rfm9x;

            can_frame_t frame1;
            memset(&frame1, 0, sizeof(frame1));
            frame1.id = 0x200;
            frame1.dlc = 8;
            frame1.data[0] = 0x10; // SPACECAN_TYPE_COMMAND
            frame1.data[1] = 0x01; // SPACECAN_CMD_RUN_TEST
            frame1.data[2] = 0x01; // Test ID

            uint16_t freq_x10 = freq_mhz * 10;
            frame1.data[3] = (freq_x10 >> 8) & 0xFF;
            frame1.data[4] = freq_x10 & 0xFF;

            frame1.data[5] = power;
            frame1.data[6] = mod;
            frame1.data[7] = bw;

            can_send(&frame1);
            printf("Sent FRAME 1 of SPACECAN RUN_TEST.\n");

            can_frame_t frame2;
            memset(&frame2, 0, sizeof(frame2));
            frame2.id = 0x201;
            frame2.dlc = 5;
            frame2.data[0] = 0x11;
            frame2.data[1] = sf;
            frame2.data[2] = cr;

            uint8_t csum = 0;
            for (int i = 0; i < 8; i++) {
                csum += frame1.data[i];
            }
            csum += frame2.data[1];
            csum += frame2.data[2];

            frame2.data[3] = csum;
            frame2.data[4] = 0x00;

            printf("Checksum for combined frames: 0x%02X\n", csum);

            can_send(&frame2);
            printf("Sent FRAME 2 of SPACECAN RUN_TEST.\n");
        }
        else if (c == '2' || c == '4')
        {
            printf("Entering parameter selection mode for %s...\n", (c == '2') ? "SX1280" : "RFM9x");

            int power_dbm = 0;
            int modulation = 1;
            int bw = 0;
            int sf = 0;
            int cr = 0;
            uint16_t freq_mhz = (c == '2') ? 2250 : 915;

            if (c == '4') {
                freq_mhz = (uint16_t)read_int_with_echo("Enter frequency in MHz (e.g. 915): ");
            }

            power_dbm = read_int_with_echo("Enter power in dBm: ");

            printf("Select modulation:\n");
            printf("1. LoRa (0x01)\n");
            printf("2. FLRC (0x03)\n");
            printf("3. GFSK (0x00)\n");
            modulation = read_int_with_echo("Selection: ");
            uint8_t modulation_reg = 0x01;
            if (modulation == 2) modulation_reg = 0x03;
            else if (modulation == 3) modulation_reg = 0x00;

            if (c == '2') {
                printf("Select bandwidth:\n");
                printf("1. 200 kHz (0x34)\n");
                printf("2. 400 kHz (0x26)\n");
                printf("3. 800 kHz (0x18)\n");
                printf("4. 1600 kHz (0x0A)\n");
                bw = read_int_with_echo("Selection: ");
            } else {
                printf("Select RFM9x bandwidth:\n");
                printf("1. 7.8 kHz (0x00)\n");
                printf("2. 62.5 kHz (0x06)\n");
                printf("3. 125 kHz (0x07)\n");
                printf("4. 250 kHz (0x08)\n");
                printf("5. 500 kHz (0x09)\n");
                bw = read_int_with_echo("Selection: ");
            }

            uint8_t bw_reg = (c == '2') ? 0x26 : 0x07;
            if (c == '2') {
                if (bw == 1) bw_reg = 0x34;
                else if (bw == 3) bw_reg = 0x18;
                else if (bw == 4) bw_reg = 0x0A;
            } else {
                if (bw == 1) bw_reg = 0x00;
                else if (bw == 2) bw_reg = 0x06;
                else if (bw == 4) bw_reg = 0x08;
                else if (bw == 5) bw_reg = 0x09;
            }

            printf("Select spreading factor:\n");
            printf("1. SF5 (0x50)\n");
            printf("2. SF6 (0x60)\n");
            printf("3. SF7 (0x70)\n");
            printf("4. SF8 (0x80)\n");
            printf("5. SF9 (0x90)\n");
            printf("6. SF10 (0xA0)\n");
            printf("7. SF11 (0xB0)\n");
            printf("8. SF12 (0xC0)\n");
            sf = read_int_with_echo("Selection: ");
            uint8_t sf_reg = 0x70;
            sf_reg = 0x40 + (sf * 0x10);

            printf("Select coding rate:\n");
            printf("1. 4/5 (0x01)\n");
            printf("2. 4/6 (0x02)\n");
            printf("3. 4/7 (0x03)\n");
            printf("4. 4/8 (0x04)\n");
            cr = read_int_with_echo("Selection: ");
            uint8_t cr_reg = (uint8_t)cr;

            if (c == '2') {
                default_power_dbm_sx1280 = (uint8_t)power_dbm;
                default_modulation_reg_sx1280 = modulation_reg;
                default_bw_reg_sx1280 = bw_reg;
                default_sf_reg_sx1280 = sf_reg;
                default_cr_reg_sx1280 = cr_reg;
            } else {
                default_power_dbm_rfm9x = (uint8_t)power_dbm;
                default_modulation_reg_rfm9x = modulation_reg;
                default_bw_reg_rfm9x = bw_reg;
                default_sf_reg_rfm9x = sf_reg;
                default_cr_reg_rfm9x = cr_reg;
                default_freq_mhz_rfm9x = freq_mhz;
            }

            printf("[INFO] Updated defaults for %s\n", (c == '2') ? "SX1280" : "RFM9x");
        }
        else if (c == '5')
        {
            printf("Entering CAN Receive Mode. Press 'q' to quit.\n");
            while (1)
            {
                if (can_receive(&frame))
                {
                    printf("Received CAN Frame: ID=0x%lX DLC=%d Data:", frame.id, frame.dlc);
                    for (int i = 0; i < frame.dlc; i++)
                    {
                        printf(" %02X", frame.data[i]);
                    }
                    printf("\n");
                }
                int quit = getchar_timeout_us(0);
                if (quit == 'q')
                {
                    printf("Exiting CAN Receive Mode.\n");
                    break;
                }
                sleep_ms(10);
            }
        }
        else
        {
            printf("Invalid option.\n");
        }
    }
}