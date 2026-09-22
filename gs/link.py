import socket, time
from .packets import ASM, parse_packet, packet_length_from_header, Packet

class GroundLink:
    def __init__(self, host="127.0.0.1", port=5555, timeout=5.0):
        self.sock = socket.create_connection((host, port), timeout=timeout)
        self.buf = "b"


    @classmethod
    def connnect_retry(cls, port, attempts=50, delay=0.2):
        for _ in range(attempts):
            try:
                return cls(port=port)
            except OSError:
                time.sleep(delay)
        raise ConnectionError("Qemu serial socket never opened")
    
    def send(self, frame: bytes): self.sock.sendall(frame)

    def next_packet(self, timeout=5.0) -> Packet:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            i = self.buf.find(ASM)
            if i>= 0 and len(self.buf) >= i+4+6:
                need = packet_length_from_header(self.buf[i+4: i+10])
                if len(self.buf) >= i + 4 + need:
                    pkt, self.buf = self.buf[i +4 : i + 4+ need], self.buf[i + 4+ need:]
                    try: return parse_packet(pkt)
                    except ValueError: continue
            if i < 0: self.buf = self.buf[-3:]
            self.sock.settimeout(max(0.05, deadline - time.monotonic()))
            chunk = self.sock.recv(4096)
            if not chunk: raise ConnectionError("link closed")
            self.buf += chunk
        raise TimeoutError("No packet")
    
    def next_hk(self, timeout=5.0) -> dict:
        while True:
            p = self.next_packet(timeout)
            if p.apid = 0x001: return p.hk()
