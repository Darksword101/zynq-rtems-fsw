def test_hk_rate_is_about_1hz(fsw):
    link, _ = fsw
    seqs = [link.next_hk()["uptime_ms"] for _ in range(5)]
    assert 3500 <= seqs[-1] - seqs[0] <= 6000
    assert all(b > a for a, b in zip(seqs, seqs[1:]))

def test_no_missed_deadlines_and_no_drops(fsw):
    link, _ = fsw
    hk = link.next_hk()
    assert hk["sensor_missed"] == 0 and hk["sb_drops"] == 0
    assert hk["sensor_max_wall_us"] < 12_000