## Simple Ping Tool in C

A lightweight clone of the `ping` command written in C using **raw sockets** and **ICMP (Internet Control Message Protocol)**. This project demonstrates how devices communicate at the network layer and helps you understand the basics of packet construction and analysis.

## Features

1. Sends ICMP Echo Request packets.
2. Receives ICMP Echo Reply packets.
3. Calculates and displays Round-Trip Time (RTT).
4. Uses raw sockets for low-level packet control.
5. Runs in a loop to mimic real `ping` behavior.

## How to Use

### Compile the Code 

```bash
gcc ping.c -o ping
```

### Run with Root Privileges

```bash
sudo ./ping <ip_address>
```
