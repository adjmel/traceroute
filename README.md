
# Ft_traceroute

This is a personal implementation of the `traceroute` command, written entirely in C as part of a UNIX networking project.

The goal of this project was to get hands-on experience with raw sockets, ICMP messages, IP routing, and system-level network debugging tools. Instead of relying on the system's built-in traceroute, I built everything from scratch — including packet crafting and response parsing.

---

## 🌍 What is Traceroute?

`traceroute` is a network diagnostic tool used to track the path that IP packets take from your machine to a destination host. It works by sending packets with increasing Time-To-Live (TTL) values and listening for ICMP "Time Exceeded" or "Destination Unreachable" replies from each hop along the route.

---

## 🛠 Features

- ✅ Written in pure C (no external libraries)
- ✅ Uses UDP sockets and ICMP response handling
- ✅ Properly handles both IP addresses and FQDNs (without DNS resolution in output)
- ✅ Displays RTTs (Round Trip Times) per probe
- ✅ Tolerates a ±30ms RTT variation (as per project specs)
- ✅ Handles invalid arguments with custom error messages
- ✅ Includes a `--help` option for usage instructions

---

## 🚫 Limitations

- ❌ Only supports IPv4 (no IPv6)
- ❌ Only manages basic usage with one destination argument
- ❌ DNS name resolution is only done for the target (not per-hop)

---

## 🔧 Allowed Functions

As per project requirements, I was limited to a specific set of system calls and libc functions, including:

- `socket`, `bind`, `recvfrom`, `sendto`
- `select`, `setsockopt`, `gettimeofday`
- `getaddrinfo`, `getnameinfo`, `inet_pton`, `inet_ntoa`
- `gethostbyaddr`, `getpid`, `exit`, `close`
- `malloc`, `free`, `read`, `write`, `printf`

---

## 🚀 Usage

```bash
$ make
$ sudo ./ft_traceroute <destination>
````

Example:

```bash
$ sudo ./ft_traceroute 8.8.8.8
```

To see usage info:

```bash
$ ./ft_traceroute --help
```

---

## 🧪 Testing

I tested the tool using both valid and invalid IPs, simulated unreachable routes, and compared the results against the official `traceroute` command to check timing and hop consistency.

---

## 🧠 What I Learned

* Low-level network programming in C
* Handling raw packet structures (IP, UDP, ICMP headers)
* Proper memory and error management in a network context
* Understanding how `traceroute` works internally (TTL, ICMP types, etc.)

