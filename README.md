# eBPF Process Monitoring Tool

## Overview
This program is a small Linux process monitoring tool built using **eBPF** and a
Python userspace program. It records basic process lifecycle information by
listening to kernel tracepoints for process execution and exit events.

---

## What the Tool Does
- Observes when a process starts (`exec`) and exits
- Collects a limited set of metadata:
  - Process ID (PID)
  - Parent Process ID (PPID)
  - Command name
  - User ID (resolved to username in userspace)
  - Runtime duration
- Writes collected events to a CSV file
- Runs until manually stopped

---

### High-Level Design
```
+---------------------+
|     Linux Kernel    |
|---------------------|
| eBPF Program        |
|  - Tracepoints      |
|  - Process metadata |
|  - Timing data      |
+----------+----------+
           |
           | Perf Buffer
           v
+---------------------+
| Userspace (Python)  |
|---------------------|
| - Load eBPF code    |
| - Process events    |
| - Enrich data       |
| - Write CSV output  |
+---------------------+
```

### Design Rationale
- **Tracepoints** are used instead of kprobes for ABI stability and reliability.
- Kernel logic is intentionally minimal.
- Data enrichment and formatting are handled in userspace.

---

### Kernel Space (eBPF)
- Hooks:
  - `sched:sched_process_exec`
  - `sched:sched_process_exit`
- Uses:
  - `BPF_HASH` to store process start timestamps
  - `BPF_PERF_OUTPUT` to send events to userspace
- Execution time is calculated using kernel timestamps via `bpf_ktime_get_ns()`.

### Userspace (Python)
- Loads the eBPF program using **BCC**
- Receives events in real time
- Resolves UID to username
- Writes structured CSV output
- Handles clean shutdown via signal handling

---



### System Overview & Requirements
- Operating System - Ubuntu 24.04.3 LTS
- Processor - Intel® Core™ i5-5200U × 4
- Windowing System - Wayland
- Kernel Version - Linux 6.14.0-37-generic
- OS Type - 64-bit
- GNOME Version - 46
- Root privileges (required for eBPF)

### Dependencies
- Python 3
- BCC
- Linux kernel headers

On Ubuntu/Debian:
```bash
sudo apt install -y bpfcc-tools linux-headers-$(uname -r) python3-bcc

```
Refer to : 
https://www.linode.com/docs/guides/how-to-install-bcc/

---

## Usage

### Run the Monitor
```bash
sudo python3 load_proc_mon.py
```

### Stop the Monitor
Press `Ctrl + C` to stop the monitor cleanly.

### Output
Captured events are written in real time to:
```
process_events.csv
```

Example CSV format:
```
PID,PPID,USER,COMMAND,DURATION_MS
1234,1200,root,bash,1520.34
```

---

## Assumptions
- Only process `exec` and `exit` lifecycle events are monitored
- Fork-without-exec events are not tracked
- User resolution is performed in userspace
- Execution time is measured from exec to exit

---


These limitations are accepted to keep the implementation simple and readable.

---


## Conclusion
This project demonstrates eBPF-based programming stepping stones for kernel–userspace interaction.

---

