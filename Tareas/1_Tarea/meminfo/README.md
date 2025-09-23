# meminfo Project

## Description
The `meminfo` project is a C application that provides a text user interface (TUI) to display system memory and CPU information. It retrieves data from system files and updates the display every 2 seconds, allowing users to monitor their system's performance in real-time.

## Features
- Displays total installed RAM.
- Shows currently used physical and virtual memory.
- Provides information about the processor model and the number of cores.
- Calculates processor load per core.

## Project Structure
```
meminfo
├── src
│   ├── main.c          # Entry point of the application
│   ├── utility.c       # Implementation of utility functions
│   └── utility.h       # Header file for utility functions
├── Makefile            # Build instructions
├── .gitignore          # Files to ignore in version control
└── README.md           # Project documentation
```

## Compilation
To compile the project, navigate to the `meminfo` directory and run the following command:

```
make
```

This will generate an executable named `meminfo`.

## Running the Application
After compiling, you can run the application with:

```
./meminfo
```

The application will start displaying system memory and CPU information, updating every 2 seconds.

## Dependencies
This project may require the `ncurses` library for the text user interface. Make sure to install it on your system before compiling.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.