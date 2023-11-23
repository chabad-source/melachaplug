[![Melacha Plug Banner](https://github.com/chabad-source/melachaplug/blob/main/images/Melacha%20Plug%20Banner.png)](https://github.com/chabad-source/melachaplug)

[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f17caa6e3d2946378de9beae9fc0ffe8)](https://www.codacy.com/gh/chabad-source/melachaplug/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=chabad-source/melachaplug&amp;utm_campaign=Badge_Grade)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate/?hosted_button_id=Q9A7HG8NQEJRU)

# Melacha Plug
A [Melacha](https://www.chabad.org/95906/) aware plug which knows when it's [Shabbos](https://www.chabad.org/633659/) or [Yom Tov](https://www.chabad.org/holidays/default_cdo/jewish/holidays.htm).

This project is using [ESPHome](https://esphome.io/).

Looking to disable a smart doorbell for Shabbos?
Worried about Google or Alexa accidentally triggering on Shabbos or Yom Tov?
The solution is here!

## Features
-   [x] Smart: It features a full Jewish calendar on board, not just for Shabbos, but also for Yom Tov!
-   [x] Failsafe: Buttons are disabled on Shabbos and Yom Tov for added security.
-   [x] Intelligent: It automatically detects your location and provides the option to further customize.
-   [x] Aware: A LED flash lets you know when Shabbos mode is enabled.
-   [x] Local: All logic is processed on board.
-   [x] Adaptable: All code is open source, allowing for easy tweaking, correction, and expansion.
-   [x] Easy: Simply plug and play, and you're all set!

![Web Server Screenshot](https://github.com/chabad-source/melachaplug/blob/main/images/Web%20Server%20Screenshot.png)

## Getting Started

### What You'll Need
-   A plug with ESPHome or Tasmota software installed.
-   Pre-flashed plugs can be bought from the [Athom](https://www.athom.tech/) or [Cloud Free](https://cloudfree.shop/) shop. There is also some preflashed plugs to be found on [Ebay](https://www.ebay.com/sch/i.html?_nkw=preflashed+smart+plug), and [Amazon](https://www.amazon.com/s?k=KAUF+Esphome).

## Installation

### Pre-configured

*Limitations*
-   Timezone is locked to EST unless you flash it yourself.
-   Your location won't be 100% accurate (you can update it via the web UI). 

*Instructions*
-   Download the melachaplug.bin file from above.
-   If your device is using Tasmota, follow instructions on how to upgrade from the [ESPHome docs](https://esphome.io/guides/migrate_sonoff_tasmota.html) or [Tasmota docs](https://tasmota.github.io/docs/Upgrading/#upgrade-using-webui).
-   If your device is using ESPHome, use the web UI to install the firmware.
-   Once the firmware is installed it will create a WiFi access point called "Melacha Plug Fallback", connect to it using any phone or computer.
-   On the devices webpage enter in your WiFi details (it should pop up, if it doesn't then it can be accessed via it's IP).
-   The device will restart, and should appear on your WiFi network.

### Custom Configuration
-   Set up your own ESPhome instance. If you have [Home Assistant](https://www.home-assistant.io/) then use the [ESPHome Dashboard](https://esphome.io/guides/getting_started_hassio.html), otherwise follow the [Command Line Interface](https://esphome.io/guides/getting_started_command_line.html) guide.
-   Find the YAML file for your device from the esphome folder above.
-   Configure the YAML settings to your liking, and flash device (more details on the ESPHome site).

## Advanced

### Using Relays
You can customize this project for any sort of relays, including the sonoff basic. 

### Shabbos Mode
Shabbos from this project can be reused to be helpful in all types of projects.

### Potential Use Cases
-   Those with motion based lights, can have the motion disabled on Shabbos and Yom Tov.
-   Preserve energy usage during Shabbos and Yom Tov, for example of water heaters (temperature should be kept above °120F to prevent [Legionnaires’ disease](https://www.cdc.gov/legionella/wmp/control-toolkit/potable-water-systems.html)).
-   Turn off security cameras.
-   Disable accidental light switch triggers on Shabbos and Yom Tov (advanced).
-   Turn on a hot water urn before Shabbos and have it auto turn off after.
-   Disable a dishwasher so it won't accidentally get turned on.


## Contribute 

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate/?hosted_button_id=Q9A7HG8NQEJRU) - or - [!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/rebbepod)
