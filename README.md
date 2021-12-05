[![Melacha Plug Banner](https://github.com/chabad-source/melachaplug/blob/main/images/Melacha%20Plug%20Banner.png)](https://github.com/chabad-source/melachaplug)

[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f17caa6e3d2946378de9beae9fc0ffe8)](https://www.codacy.com/gh/chabad-source/melachaplug/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=chabad-source/melachaplug&amp;utm_campaign=Badge_Grade)

# Melacha Plug
A [Melacha](https://www.chabad.org/95906/) aware plug which turns off when it's [Shabbos](https://www.chabad.org/633659/) or [Yom Tov](https://www.chabad.org/708510/).

This project is using [ESPHome](https://esphome.io/).

Looking to disable a smart doorbell for Shabbos?
Worried about Google or Alexa listening to you on Shabbos or Yom Tov?
The solution is here!

## Features
-   [x] Smart: A full Jewish Calendar onboard, it knows when Yom Tov is. 
-   [x] Failsafe: Buttons will be disabled on Shabbos. 
-   [x] Intelligent: Auto detects your location with the option to customize. 
-   [x] Aware: LED flash lets you know when Shabbos mode is enabled.
-   [x] Local: All logic is done on-board.
-   [x] Adaptable: All code is open source, tweak, correct and expand.
-   [x] Easy: Simply plug and play, your all set!

## Getting Started

### What You'll Need
-   A plug with ESPHome or Tasmota software installed.
-   Pre-flashed plugs can be bought from the [Athom](https://www.athom.tech/) or [Cloud Free](https://cloudfree.shop/) shop.

## Installation

### Pre-configured

*Limitations*
-   Timezone won't be set to your specific location.
-   Your location won't be 100% accurate. 

-   Download the melachaplug.bin file from above.
-   If your device is using Tasmota, follow instructions on how to upgrade from the [ESPHome docs](https://esphome.io/guides/migrate_sonoff_tasmota.html) or [Tasmota docs](https://tasmota.github.io/docs/Upgrading/#upgrade-using-webui).
-   If your device is using ESPHome, use the web UI to install the firmware.

### Custom Configuration
-   Set up your own ESPhome instance. If you have [Home Assistant](https://www.home-assistant.io/) then use the [ESPHome Dashboard](https://esphome.io/guides/getting_started_hassio.html), otherwise use their [Command Line Interface](https://esphome.io/guides/getting_started_command_line.html) guide.
-   Configure the YAML settings to your liking, and flash device (more details on the ESPHome site).

## Advanced

### Using Relays
You can customize this project for any sort of relays, includung the sonoff basic. 

### Shabbos Mode
Shabbos from this project can be reused to be helpful in all types of projects.

### Potential Use Cases
-   Those with motion based lights, can have the motion disabled on Shabbos and Yom Tov.
