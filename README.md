# Shared Memory IPC

## Overview

This project implements bidirectional communication between two seperate processes using shared memory in C++.

## Features

- **Bidirectional Communication**: Both processes are able to send commands to each other.
- **Mutex for Thread Safety**: By performing mutex locks we ensure safe access to shared resources.
- **Event Based**: We use events to signal when events are ready to be performed.

![image](https://i.imgur.com/NrXU9V8.png)
