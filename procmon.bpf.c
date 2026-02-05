#include <uapi/linux/ptrace.h>
#include <linux/sched.h>

struct event_t {
    u32 pid;
    u32 ppid;
    u32 uid;
    char comm[TASK_COMM_LEN];
    u64 duration_ns;
};

BPF_HASH(start_times, u32, u64);
BPF_PERF_OUTPUT(events);

TRACEPOINT_PROBE(sched, sched_process_exec) {
    u32 pid = bpf_get_current_pid_tgid() >> 32;
    u64 ts = bpf_ktime_get_ns();
    start_times.update(&pid, &ts);
    return 0;
}

TRACEPOINT_PROBE(sched, sched_process_exit) {
    struct event_t event = {};
    u32 pid = bpf_get_current_pid_tgid() >> 32;

    u64 *start = start_times.lookup(&pid);
    if (!start) {
        return 0;
    }

    struct task_struct *task;
    task = (struct task_struct *)bpf_get_current_task();

    event.pid = pid;
    event.ppid = task->real_parent->tgid;
    event.uid = bpf_get_current_uid_gid();
    bpf_get_current_comm(&event.comm, sizeof(event.comm));
    event.duration_ns = bpf_ktime_get_ns() - *start;

    events.perf_submit(args, &event, sizeof(event));
    start_times.delete(&pid);
    return 0;
}
