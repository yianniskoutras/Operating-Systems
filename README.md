# Operating Systems Lab Exercises

This repository contains several C programs demonstrating process control, inter-process communication, signal handling, and networking.

## Files

- **lab1.c**  
  Demonstrates basic forking and file operations. The child writes its PID info to a file, and the parent writes its own after waiting.

- **parent_test.c**  
  Creates multiple child processes, manages them with signals, and uses pipes for communication. It also respawns children upon termination.

- **child.c**  
  Works with parent_test.c; it handles signals to toggle its state and prints gate status messages.

- **Makefile**  
  Contains build instructions for the programs.

- **ask3a.c**  
  Implements multi-process job scheduling using pipes. It assigns jobs to children in either a round-robin or random fashion.

- **ask4.c**  
  A TCP client that connects to a server, sends commands, and processes responses (e.g., event info, temperature, light levels).
