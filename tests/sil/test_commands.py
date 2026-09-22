from gs import packets as P
def test_noop_increment_accept(fsw):
    link, _ = fsw
    before = link.next_hk()["cmd_accept"]; link.send(P.cmd_noop())
    assert any(link.next_hk()["cmd_accept"] == before + 1 for _ in range(2))  #allow one HK period of latency

def test_bad_crc_rejected(fsw):
    link, _ = fsw
    bad = bytearray(P.cmd_noop()); bad[-1] ^= 0xFF
    before = link.next_hk()["cmd_reject"]; link.send(bytes(bad))
    assert any(link.next_hk()["cmd_reject"] == before + 1 for _ in range(2))

def test_set_tlm_period(fsw):
    link, _ = fsw
    link.send(P.cmd_set_tlm_period(250))
    assert any(link.next_hk()["tlm_period_ms"] == 250 for _ in range(3))
    link.send(P.cmd_set_tlm_period(1000))
    assert any(link.next_hk()["tlm_period_ms"] == 1000 for _ in range(3))