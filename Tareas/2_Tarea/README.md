# Embedded Session Homework

This project implements a simple closed-loop controller for an embedded system, simulating sensor readings and actuator control.

## Project Structure

- `Makefile`: Build scripts for 64-bit and 32-bit binaries.
- `README.md`: This file.
- `sensor/`: Contains sensor interface and implementation.
- `actuators/`: Contains polymorphic actuator interface and LED/Buzzer implementations.
- `controller/`: Implements the main control logic.
- `tests/`: Contains `sensor_feed.csv` for simulated sensor data.

## Requirements Implemented

1.  **Folder Structure**: As specified.
2.  **Include Guards**: All header files (`.h`) include standard include guards.
3.  **`sensor.h` / `sensor.c`**: Declares `sensor_init` and `sensor_read`. `sensor.c` reads values from `tests/sensor_feed.csv`.
4.  **`actuator.h`**: Declares a polymorphic actuator interface (`actuator_interface_t`) with `params`, `activate`, `deactivate`, and `status` function pointers.
5.  **`led_actuator.c` / `buzzer_actuator.c`**: Implement the `actuator_interface_t`. For simulation, they log their state changes.
6.  **`ctl.c`**: Implements the closed-loop controller:
    *   Samples sensor every 100 ms.
    *   Activates/deactivates LED/Buzzer based on a threshold.
    *   Uses `clock_gettime(CLOCK_MONOTONIC)` for time.
    *   Logs timestamp, sensor value, and actuator states.
7.  **Makefile Targets**: `make ctl64`, `make ctl32`, `make clean`.
8.  **Compilation Flags**: `gcc -Wall -Wextra -std=c11` is used in the Makefile.
9.  **Placeholders**: `README.md` and `tests/sensor_feed.csv` are provided.

## Build Instructions

To build the project, navigate to the `embedded-session-hw/` directory and use one of the following `make` commands:

-   **Build 64-bit executable:**
    ```bash
    make ctl64
    ```
    This will create an executable named `ctl64` in the root directory.

-   **Build 32-bit executable:**
    ```bash
    make ctl32
    ```
    *Note: This requires `gcc-multilib` to be installed on your system. On Debian/Ubuntu, you can install it using `sudo apt-get install gcc-multilib`.*
    This will create an executable named `ctl32` in the root directory.

-   **Clean build artifacts:**
    ```bash
    make clean
    ```
    This removes all generated object files (`.o`) and executables (`ctl64`, `ctl32`).

## Running the Controller

After building, you can run the controller:

```bash
./ctl64