import struct
from dataclasses import dataclass
from .crc16 import crc16_ccitt

ASM = b"\x1a\xcf\xfc\x1d"
HK_FMT = "<IBBHHHIIfffffffIHH"
HK_FIELDS = ("uptime_ms mode fault_flags cmd_accept cmd_reject sensor_missed sample_count sb_drops "
             "gx gy gz ax ay az temp_c sensor_max_wall_us tlm_period_ms reserved").split()
assert struct.calcsize(HK_FMT) == 56
APID_HK, APID_EVENT = 0x001, 0x002
MODE_NAMES = {0: "BOOT", 1: "NOMINAL", 2: "SAFE"}

@dataclass
class Packet:
    apid: int; seq: int; time_ns: int; payload: bytes
    def hk(self) -> dict: return dict(zip(HK_FIELDS, struct.unpack(HK_FMT, self.payload)))
    def event(self) -> str: return self.payload.split(b"\0", 1)[0].decode(errors="replace")

def parse_packet(pkt: bytes) -> Packet:
    w0, w1, dlen = struct.unpack(">HHH", pkt[:6])
    total = 6 + dlen + 1
    if len(pkt) < total: raise ValueError("short")
    if crc16_ccitt(pkt[:total - 2]) != struct.unpack(">H", pkt[total - 2:total])[0]: raise ValueError("crc")
    time_ns, = struct.unpack("<Q", pkt[6:14])
    return Packet(apid=w0 & 0x7FF, seq=w1 & 0x3FFF, time_ns=time_ns, payload=pkt[14:total - 2])

def packet_length_from_header(hdr6: bytes) -> int:
    return 6 + struct.unpack(">H", hdr6[4:6])[0] + 1

def build_command(opcode: int, payload: bytes = b"") -> bytes:
    body = bytes([opcode, len(payload)]) + payload
    return b"\xeb\x90" + body + struct.pack(">H", crc16_ccitt(body))

# convenience encoders
def cmd_noop(): return build_command(0x00)
def cmd_set_tlm_period(ms: int): return build_command(0x02, struct.pack("<H", ms))
def cmd_set_temp_limit(c: float): return build_command(0x03, struct.pack("<f", c))
def cmd_enter_safe(): return build_command(0x04)
def cmd_exit_safe(): return build_command(0x05)
def cmd_inject_fault(delta_c: float): return build_command(0x06, struct.pack("<f", delta_c))