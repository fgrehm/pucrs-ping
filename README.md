# pucrs-ping

A (rudimentary) `ping` implementation in C built for the PUCRS Networking II 2015.2 course

## Usage

1. Figure out your network's gateway IP, run `arp` and grab its MAC address.
2. Find out what is your IP address, MAC address and network interface index with `ip addr`.
3. Change the interface name and index on the `constants.h` file.

```sh
make && \
sudo ./pucrs-ping \
      'e8:b1:fc:00:5d:f2' \
      '192.168.0.12' \
      '28:32:c5:d4:47:8a' \
      '192.168.0.1'
```
