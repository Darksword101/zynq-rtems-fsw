import argparse, sys
from .link import GroundLink
from . import packets as P
from .packets import MODE_NAMES

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=5555)
    ap.add_argument("--cmd", choices=["noop","reset","enter-safe","exit-safe","inject","tlm-period","temp-limit"])
    ap.add_argument("--value", type=float)
    a = ap.parse_args()
    link = GroundLink(port = a.port)
    if a.cmd:
        frame = {"noop": P.cmd_noop, "reset": lambda: P.build_command(0x01), "enter-safe": P.cmd_enter_safe,
                 "exit-safe": P.cmd_exit_safe, "inject": lambda: P.cmd_inject_fault(a.value),
                 "tlm-period": lambda: P.cmd_set_tlm_period(int(a.value)), "temp-limit": lambda: P.cmd_set_temp_limit(a.value)}[a.cmd]()
        link.send(frame);
        print(f"sent {frame.hex()}")
    while True:
        p = link.next_packet()
        if p.apid == P.APID_HK:
            h = p.hk()
            print(f"[HK  #{p.seq:5d}] t={h['uptime_ms']/1000:8.2f}s mode={MODE_NAMES[h['mode']]:7s} "
                  f"faults=0x{h['fault_flags']:02x} temp={h['temp_c']:6.2f}C acc={h['cmd_accept']} rej={h['cmd_reject']} "
                  f"missed={h['sensor_missed']} drops={h['sb_drops']} maxwall={h['sensor_max_wall_us']}us")

        elif p.apid == P.APID.EVENT:
            print(f"[EVT #{p.seq:5d}] {p.event()}")
if __name__ == "__main__": main()