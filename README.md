

# 🚀 SpaceCAN Test Console

**A command-line tool for testing CubeSat radio payloads over CAN bus.**

This CLI app lets you send test commands to your CubeSat radio payload systems (like the SX1280 and RFM9x radios). It’s perfect for **bench testing**, verifying hardware, and learning how your radio stack works.

---

## ✨ What This Tool Does

✅ Send commands over CAN bus to:

- **SX1280 radio** (2.4 GHz, S-band)
- **RFM9x radio** (900 MHz band)

✅ Choose transmission parameters like:

- Frequency
- Power level
- Bandwidth
- Spreading Factor
- Coding Rate

✅ Listen for CAN frames from the payload

✅ Great for:

- Learning how CubeSat payloads communicate
- Debugging hardware setups
- Experimenting with different radio settings

---

## 🔧 How It Works

When you run the CLI:

- You see a menu:
    ```
    1. Send sx1280 transmission via spaceCAN
    2. Set sx1280 transmission parameters
    3. Send rfm9x transmission via spaceCAN
    4. Set rfm9x transmission parameters
    5. Enable Receive Mode
    ```

- Choose an option:
    - Options 1 & 3 send **default test packets** over CAN
    - Options 2 & 4 let you pick parameters interactively
    - Option 5 puts the tool into **CAN receive mode** so you can see incoming frames

Under the hood:

- The tool builds a **spaceCAN protocol frame**
- Sends the frame over SPI to the MCP2515 CAN controller
- Your CubeSat payload interprets the message and transmits RF

---

## 📁 Project Structure

```
src/
    cli.c         → main CLI logic
    can_driver.c  → handles low-level CAN communication
include/
    cli.h
    can_driver.h
```

- `cli.c` → this is the heart of the user interface
- CAN communication handled via MCP2515 driver

---

## 🚀 Running The Tool

### Build it

Make sure you have your Pico SDK and toolchain installed.

```bash
mkdir build
cd build
cmake ..
make
```

### Flash to your board

Copy the `.uf2` file to your Pico.

---

## ✅ Example Session

```
CANBus Master CLI Ready.

Menu:
1. Send sx1280 transmission via spaceCAN
2. Set sx1280 transmission parameters
3. Send rfm9x transmission via spaceCAN
4. Set rfm9x transmission parameters
5. Enable Receive Mode
Select option:
```

→ Choose **3** and you’ll see logs like:

```
Sending SPACECAN RUN_TEST frame (RFM9x radio)...
Sent FRAME 1 of SPACECAN RUN_TEST.
Sent FRAME 2 of SPACECAN RUN_TEST.
```

And your CubeSat radio **transmits RF**!

---

## 💡 Why “SpaceCAN”?

This test tool uses a simple protocol we call **spaceCAN**:

- Built on top of CAN bus
- Custom frame structure
- Allows sending:
    - Radio configuration commands
    - Payload data for transmission

---

## 👩‍🚀 For Students

✅ Don’t worry if you’ve never touched CAN bus before. Here’s what this CLI does:

- Talks to your radio over CAN
- Makes it transmit test signals
- Helps debug the radio and the CAN link

Think of it as your **CubeSat radio remote control!**

---

## 🛠️ TODOs

- Add more automated tests
- Support more radio types
- Improve error handling