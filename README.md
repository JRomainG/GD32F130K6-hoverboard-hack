# GD32 F130K6 Hoverboard hack

This repository shows the steps to unlock the hoverboard sideboards and flash a firmware. No firmware is included, but the amazing work from [EmanuelFeru](https://github.com/EmanuelFeru?tab=repositories) can be reused.

## Identifying the hoverboard

Multiple different hoverboard models exist, you should find the right repository for your device.
Here, we are using ([this board](https://www.amazon.fr/gp/product/B07P1Y83MG/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1)), which needs as a specific procedure to flash it, described below.

* For older models with only one motherboard, see [the FOC repository](https://github.com/EmanuelFeru/hoverboard-firmware-hack-FOC),
* For models with 2 sideboards, the chip must be identified:
  * If the chip is an STM32 (see [this picture](https://raw.githubusercontent.com/EmanuelFeru/hoverboard-sideboard-hack-STM/master/docs/pictures/sideboard_pinout.png)), follow the [STM repository](https://github.com/EmanuelFeru/hoverboard-sideboard-hack-STM).
  * If the chip is a GD32 F130C6 (see [this picture](https://raw.githubusercontent.com/EmanuelFeru/hoverboard-sideboard-hack-GD/master/docs/pictures/sideboard_pinout.png)), follow the [GD repository](https://github.com/EmanuelFeru/hoverboard-sideboard-hack-GD),
  * If the chip is a GD32 F130K6 (see [the open hoverboard](images/open_hoverboard.jpg), [the sideboard](images/sideboard.jpg), and [a closeup of the chip](images/GD32F130K6_chip.jpg)), continue reading.

## Hardware

The sideboard has a 4-pin debugging header with a 3V3, SWDIO, SWCLK, and GND input accessible next to the chip:

![Hoverboard master sideboard](images/sideboard.jpg)

The chip itself is a GD32 F130K6, for which you can find a datasheet [here](doc/GD32F130xx_Datasheet_Rev3.1.pdf):

<p float="left">
  <img alt="Hoverboard master sideboard" src="images/GD32F130K6_chip.jpg" width="49%"/>
  <img alt="GD32F130K6 pin mapping" src="images/GD32F130K6_datasheet_pins.png" width="49%"/>
</p>

**Important:** The "dot" on the datasheet's schematics used to indicate the orientation is located at the *bottom left* of the chip when the text is upright, contrary to what you might expect.

**Helpful notes:**

* We recommend plugging a [pin header](https://en.wikipedia.org/wiki/Pin_header) to make debugging and flashing easier, as well as testing faster (in the improbable case where you don't succeed first try). It also makes the pins accessible when the PCB is fully assembled on the chassis.
* Put a piece of tape on the speaker of the slave board, this just might save your ears during the hacking process.
* The motherboards detect the orientation of the hoverboard for safety reasons, so you may find yourself trying to unlock the chip but instead making the board beep, and being unable to turn it off without unplugging the battery. Keep in mind that you can reset the board by pushing the on/off button once, then pressing it for 5 secondes while the hoverboard is leveled.

**TODO:** Add a picture of the board with the debug pins and the of tape.

Here are some of the pins located on a real board:

![Hoverboard master sideboard](images/GD32F130K6_pins.jpg)

Since the 3V3, GND, SWDIO and SWCLK are already connected to the debugging header, there is no need to bother with them. However, correctly identifying the NRST pin is important to unlock the board.

## Flashing

For this setup, you can completly disconnect the sideboards from the rest of the equipment, including the battery. We recommend working on them while totaly disconnected from any hardware for practicality. You must have access to an ST-LINK v2 like the following:

![ST-LINK v2](images/stlink.jpg)

Connect the 3V3, SWDIO, SWCLK, and GND pins from the ST-LINK to the debugging header on the sideboard.

### Unlocking

If this is your first time flashing the board, it must be unlocked.

At this points, you can plug the ST-LINK in your computer and use [ST-LINK utility](https://www.st.com/en/development-tools/stsw-link004.html) to connect to it. However, the GD32 itself will most likely not respond, resulting in an error. You can try [EmanuelFeru's tutorial](https://github.com/EmanuelFeru/hoverboard-firmware-hack-FOC/wiki/How-to-Unlock-MCU-flash) on how to unlock it. If this works, good for you, you can skip the rest of these steps! Otherwise, keep on reading.

Now, the NRST pin must be taken care of. Because it is not connected to the debugging header, you need to find a way to manually connect a cable. It is considerably easier to access the first component to which the pin is connected than the pin itself, as it is bigger and there is less risk of creating a short.

NRST stands for "Not ReSeT", which means that if the ST-Link sends a '1' bit on the RST pin, the NRST pin on the GD32 must receive a 0 instead (and vice versa). To do so, we used an Arduino Mega with a very simple program:

```C
void setup() {
  pinMode(40, INPUT);
  pinMode(A0, OUTPUT);
}

void loop() {
  int rst = digitalRead(40);
  analogWrite(A0, 1023 * !rst);
}
```

Because the output is 5.5V, and we can't use PWM considering our use case, we added a simple voltage divider to match the GD32's expected 3.3V amplitude on the NRST pin.

**TODO:** Add a diagram showing our setup and the voltage divider.

**Note:** The same result could be achieved in an easier fashion by using simple transistors, but we used what was handy. It would also probably reduce the delay between the ST-LINK and the GD32, which might make the whole procedure more reliable.
Our procedure isn't plug'n'play'n'it'works, you may have to try few times (for us, less than 10 times) the following procedure to get results, but it works.

Here's a look at our quick and dirty setup:

![Setup to reset the GD32 chip](images/reset_setup.jpg)

Now that the 3V3, SWDIO, SWCLK, GND, and NRST pins are connected, we can try unlocking the GD32.

We used [OpenOCD](http://openocd.org) to launch commands and check if our reset was working correctly:

```bash
./openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c init -c "reset halt" -c "stm32f1x unlock 0" -d
```

**Note:** If you're running Windows, you might need to start `cmd` as administrator to access the USB ports. On Linux / macOS, `sudo` might be necessary, but the error messages should be clear enough to figure it out.

The first few times, we ran into errors such as:

```
Error: init mode failed (unable to connect to the target)
```

```
stlink_usb.c:515 jtag_libusb_bulk_transfer_n(): ERROR, transfer 0 failed, error -1
stlink_usb.c:515 jtag_libusb_bulk_transfer_n(): ERROR, transfer 1 failed, error -1
```

```
stlink_usb.c:1679 stlink_usb_idcode(): IDCODE: 0xFFFFFFFF
hla_interface.c:93 hl_interface_init_target(): UNEXPECTED idcode: 0xffffffff
hla_interface.c:95 hl_interface_init_target(): expected 1 of 1: 0x1ba01477
```

```
stlink_usb.c:788 stlink_usb_error_check(): unknown/unexpected STLINK status code 0x5
```

However, we were able to successfully run the OpenOCD command with the following procedure:

* **Step 1:** Unplug the ST-LINK from your computer so the GD32 is unpowered.
* **Step 2:** Plug the NRST pin with the inverter setup.
* **Step 3:** Powerup the ST-LINK by plugging it to your computer and wait a few seconds.
* **Step 4:** Run the OpenOCD command.
* **Step 5:** Unplug the NRST pin.
* **Step 6:** Run the command again.

The goal here is to send the OpenOCD command at the right timing in the GD32 starting procedure. If it doesn't work, try again but execute **Step 5** before **Step 4**.

You can also try executing **Step 5** while the OpenOCD command is running. It's all about having the right timing. As this isn't very reproducible, it hopefully won't be necessary.

If it works, OpenOCD should keep running and you should be able to spot the following message:

```
options.c:63 configuration_output_handler(): stm32x unlocked.
```

Once you're able to run the OpenOCD command, you can follow [EmanuelFeru's tutorial](https://github.com/EmanuelFeru/hoverboard-firmware-hack-FOC/wiki/How-to-Unlock-MCU-flash) using ST-Link utility if need-be (your GD32 may already be unlocked thanks to the OpenOCD command). Make sure to stop OpenOCD before following the tutorial or both programs might conflict with one another.

**Note:** Unlocking your GD32 only needs to be done once. Afterwards, you no longer need to fiddle with the NRST pin and the whole inverter: the 3V3, SWDIO, SWCLK, and GND pins should be enough.

### Flashing

The hard part is done, now you're in known territory. The easiest was of doing things is following [EmanuelFeru's tutorial](https://github.com/EmanuelFeru/hoverboard-sideboard-hack-GD#flashing) with the associated firmware.

### Connection

**TODO:** Add picture/diagram of the board and connections. Most are all common sense but the ADC and USART require special connections.

For the ADC to work, you must change the link cable between boards from the 4-pin blue plug to the 4-pin white plug next to it (see diagram). You can then use the pin labeled in the diagram for the ADC input.

For the USART to work, you CANNOT use the labeled tx/rx empty vias on the board due to them being tied to the bldc output. You must use the one labeled in the diagram.

**Note:** For my arduino to receive correct tx transmission, you must jumper or bridge the resistor on the tx output line, labeled in the diagram.

## Resources

* [EmanuelFeru's hoverboard repository](https://github.com/EmanuelFeru/hoverboard-sideboard-hack-GD).
* [Hooover Telegram group](https://t.me/joinchat/BHWO_RKu2LT5ZxEkvUB8uw).
* [Phil Malone's "Hoverboards for Assistive Devices" post](https://hackaday.io/project/170932-hoverboards-for-assistive-devices).
* [Niklas Fauth's hoverboard repository](https://github.com/NiklasFauth/hoverboard-firmware-hack).

## Special thanks

* Candas for their help on handling the GD32 chip.
* Emanuel Feru for their amazing work on different boards.
* All the folks at [Hooover](https://t.me/joinchat/BHWO_RKu2LT5ZxEkvUB8uw) for their advice and ideas.
