import time
def test_hk_rate_is_about_1hz(fsw):
    link, _ = fsw
    t0 = time.monotonic();
    seqs = [link.next_hk()["uptime_ms"] for _ in range(5)]
    assert 3.5 < time.monotonic() - t0 < 6.0
    assert all(b - a for a, b in zip(seqs, seqs[1:]))

def test_no_missed_deadlines_and_no_drops(fsw):
    link, _ = fsw
    hk = link.next_hk()
    assert hk["sensor_missed"] == 0 and hk["sb_drops"] == 0
    link.send(P.cmd_set_tlm_period(1000))