# Hardware

To use the Trackuino-v2 code on your Arduino, you can either order a print of the PCB designed for your board type, and solder the necessary components onto the board, or you can use the schematics below to wire up your Arduino with a breadboard, protoboard, or something of that type.

## Components

You will need the following components if you choose to solder it onto the PCB. Check the notes for each component to see if alternatives are recommended for breadboard wiring.

| Label | Qty | Component | Price | Provider | Notes |
|---|---|---|---|---|---|
| A1 | 1 | Arduino | Depends on what Arduino you go with | [Arduino Store](https://store.arduino.cc/) |  |
| U1 | 1 | Radiometrix HX1 | $50 | [Uptronics](https://store.uputronics.com/index.php?route=product/product&product_id=63), [Radiometrix](https://radiometrix.mybigcommerce.com/hx1-vhf-narrow-band-fm-300mw-transmitter-standard-frequencies-144-390-144-800-and-169-4125mhz/) |  |
| J1 | 1 | SMA Female Connector | $1.40 | [Mouser](https://www.mouser.com/ProductDetail/LPRS/SMA-CONNECTOR?qs=j%252B1pi9TdxUYkOiITvzJM8A%3D%3D), [Uptronics](https://store.uputronics.com/index.php?route=product/product&path=63&product_id=79) |  |
|  | 1 | SMA Flexible Extender | $10 | [Uptronics](https://store.uputronics.com/index.php?route=product/product&path=63&product_id=58), [Mouser](https://www.mouser.com/ProductDetail/Amphenol-RF/135109-02-06.00?qs=3WadRV20yGyRMmeZhUPdRQ%3D%3D), [B&H](https://www.bhphotovideo.com/c/product/961489-REG/startech_rpsma10mf_10_rp_sma_to_rp_sma.html) | If antenna is attached straight to board, no flexible extender is needed. |
|  | 1 | Antenna | $40 | [MFJ](https://mfjenterprises.com/products/mfj-1717s) |  |
| U2 | 1 | GPS | $12 | [Amazon](https://www.amazon.com/Microcontroller-Compatible-Sensitivity-Navigation-Positioning/dp/B07P8YMVNT) | Any GPS module will work, so long as it is 5V, and outputs NMEA strings. If you are using the PCB design, having one with TX RX GND and VIN all in a row will allow it to be plug-and-play without any wire re-routing. When purchasing a GPS module, please make note of the GPS limitations on speed and altitude. |
| U3 | 1 | Bi-Directional Logic Level Converter | $3.50 | [SparkFun](https://www.sparkfun.com/products/12009) | On a breadboard, you could skip this step and instead build voltage dividers for the RX and TX lines to step them down from 5V to 3.3V. |
| U4 | 1 | OpenLog | $16.95 | [SparkFun](https://www.sparkfun.com/products/13712) |  |
| U5 | 1 | 3.3V LDO | $0.67 | [Mouser](https://www.mouser.com/ProductDetail/STMicroelectronics/LD1117S33TR?qs=edoyzKMbmhntQZx4BFmoqw%3D%3D) | Any 3.3V through-hole LDO *should* work for a breadboard, just make note of pin orientations. A LD1117V33 with a footprint of SOT223 is what you are looking for. Here is an example on [Mouser](https://www.mouser.com/ProductDetail/STMicroelectronics/LD1117V50?qs=arR7071Fstdq5MS%2FejBvyQ%3D%3D) |
| U6 | 1 | 5V LDO | $0.67 | [Mouser](https://www.mouser.com/ProductDetail/STMicroelectronics/LD1117S50TR?qs=eQN2Ig5lfEwsh0kTXN%2FkTg%3D%3D) | Any 5V through-hole LDO *should* work for a breadboard, just make note of pin orientations. A LD1117V50 with a footprint of SOT223 is what you are looking for. |
| C1-C3 | 3 | 0.1uF | $0.14 | [Mouser](https://www.mouser.com/ProductDetail/KEMET/C0603C104K3RAC7013?qs=nAEW9fCjKd89XJLxiep6YQ%3D%3D) | Any 0.1uF Capacitor should work.  |
| SW1 | 1 | Push Button | $2.50 for 20 | [Adafruit](https://www.adafruit.com/product/367) | You can pretty safely avoid installing this at all and just use the reset button on your Arduino. |
| BZ1 | 1 | Buzzer (active or passive) | $0.30 | [Active Buzzer - Addicore](https://www.addicore.com/Active-Buzzer-5V-p/ad146.htm), [Passive Busser - Addicore](https://www.addicore.com/Passive-Buzzer-p/ad319.htm) | Active buzzer is more versatile and therefore recommended.  |
| U7 | 2 | TMP36 Temperature Sensor | $1.60 | [SparkFun](https://www.sparkfun.com/products/10988) | One is to be hooked up to terminal blocks to be wired outside of your Trackuino-v2's enclosure, the other is for inside said enclosure. |
| P1 | 1 | 2-Wide Terminal Blocks | $0.65 | [Mouser](https://www.mouser.com/ProductDetail/CUI-Devices/TB002-500-02BE?qs=vLWxofP3U2x9716kcgva%2Fw%3D%3D) | Any terminal blocks will work, screwed or not, as long as they fit the same pin dimensions. Alternatively you can skip terminal blocks entirely and solder to the holes they leave on the board. Terminal blocks shouldn't be purchased for a breadboard. |
| U8 | 1 | 3-Wide Terminal Blocks | $0.66 | [Mouser](https://www.mouser.com/ProductDetail/CUI-Devices/TBL004V-508-02BE-2GY?qs=UXgszm6BlbFJXFFgC%2FsssQ%3D%3D) | Same as previous terminal block note. |
| D1-D3 | 3 | LEDs | $4 | [Amazon](https://www.amazon.com/JABINCO-Circuit-Assorted-Science-Experiment/dp/B0827KYRFH/ref=sr_1_8?crid=QILAHDHBQAH3&keywords=through-hole+leds&qid=1660157838&sprefix=through-ho%2Caps%2C866&sr=8-8) | Just any 3 LEDs. Different colors for each makes it easier to differentiate which status LED is which. |
| R1-R3 | 3 | 330 Ohm Resistors | $1.05 | [SparkFun](https://www.sparkfun.com/products/14490) | Other resistances would work too, 330 is not an exact requirement. |


## Wiring

