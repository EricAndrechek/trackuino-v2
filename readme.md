# Trackuino-v2 <!-- omit in toc -->

> [!WARNING]
> This repository/project has quickly grown in scope to eclipse the original Trackuino project. As such, it **has been moved to its own repository [here](https://github.com/EricAndrechek/umich-balloons).**
>
> This repository will be kept purely as a reference to the original project and as a way for people to find the new project from Trackuino.

An open-source high-altitude balloon tracker, based on the original [Trackuino](https://github.com/trackuino/trackuino).

This repository contains the code for the [ground station](./ground-station/), the [high-altitude balloon tracker](./balloon-tracker/), [vehicle trackers](./vehicle-tracker/), and [handheld locators](./handheld-locator/).

Designed and built by @EricAndrechek and @jfwoods under the [GPL-2 License](./license).

## Table of Contents <!-- omit in toc -->

# 1. Balloon Tracker

The balloon tracker, "payload", high-altitude balloon (hab), or Trackuino, was the original project that this repository was built around. It is a high-altitude balloon tracker, designed to be as cheap as possible, and to be easily built by anyone with a soldering iron and a few hours to spare. It is based on the original [Trackuino](https://github.com/trackuino/trackuino) although it has been heavily modified and improved. See the [balloon-tracker](./balloon-tracker/) page for more information.

# 2. Vehicle Tracker

The vehicle tracker is a small, low-power, low-cost tracker designed to be placed on chase vehicles so that their locations can be tracked by other chase teams. It uses ARPS-IS to report its location but includes no actual radio hardware, thus keeping its cost down and its power consumption low. See the [vehicle-tracker](./vehicle-tracker/) page for more information.

# 3. Handheld Locator

The handheld locator is a small and lightweight device used to track the balloon after it has landed or during its decent. It uses a different, shorter distance radio frequency to communicate two-way with the balloon tracker. Once in range, you are also able to send a kill message to the balloon tracker, which will trigger it to cut its burn wire and any other payloads on the balloon. See the [handheld-locator](./handheld-locator/) page for more information.

# 4. Ground Station

