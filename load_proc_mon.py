from bcc import BPF
import csv
import pwd
import signal
import sys

bpf = BPF(src_file="procmon.bpf.c")

output_file = open("process_events.csv", "w", newline="")
csv_writer = csv.writer(output_file)
csv_writer.writerow(["PID", "PPID", "USER", "COMMAND", "DURATION_MS"])

def handle_event(cpu, data, size):
    event = bpf["events"].event(data)
    try:
        user = pwd.getpwuid(event.uid).pw_name
    except KeyError:
        user = str(event.uid)

    csv_writer.writerow([
        event.pid,
        event.ppid,
        user,
        event.comm.decode(),
        event.duration_ns / 1e6
    ])

bpf["events"].open_perf_buffer(handle_event)

def shutdown(sig, frame):
    print("\nStopping monitor...")
    output_file.close()
    sys.exit(0)

signal.signal(signal.SIGINT, shutdown)

print("Process monitor started. Press Ctrl+C to stop.")
while True:
    bpf.perf_buffer_poll()
