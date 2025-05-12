# Unix Utilities Implementation

This repository contains implementations of simple Unix utilities (`pwd`, `echo`, `cp`, `mv`) as part of a coding assignment. Each utility is implemented in C, adhering to the specified requirements and limitations.

## Table of Contents
- [Description](#description)
- [Features](#features)
- [Files](#files)
- [Usage](#usage)
- [Requirements](#requirements)
- [Installation](#installation)
- [Limitations](#limitations)
- [Contributing](#contributing)
- [License](#license)

## Description
This project implements basic versions of the following Unix commands:
- `pwd`: Prints the current working directory.
- `echo`: Prints a user-provided string to stdout.
- `cp`: Copies a source file to a destination file.
- `mv`: Moves or renames a source file to a destination.

These implementations were developed as part of a Udemy course exercise, focusing on system calls, error handling, and matching Unix utility behavior.

## Features
- Implements `pwd_main()`, `echo_main()`, `cp_main()`, and `mv_main()` functions as per assignment requirements.
- Includes error checking for system calls and library routines.
- Matches the basic behavior of original Unix utilities on a Linux machine.
- Written in C with standard libraries (`stdio.h`, `unistd.h`, etc.).

## Files
- `pwd_main.c`: Contains the `pwd_main()` function to print the current working directory.
- `echo_main.c`: Contains the `echo_main()` function to print command-line arguments.
- `cp_main.c`: Contains the `cp_main()` function to copy a file.
- `mv_main.c`: Contains the `mv_main()` function to move or rename a file.
- `README.md`: This file, providing project documentation.

## Usage
To use each utility, compile the respective C file and run the resulting executable with appropriate arguments:

### pwd
```bash
gcc -o pwd pwd_main.c
./pwd
