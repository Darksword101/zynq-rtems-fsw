from gs import packets as P
def test_fault_injection_enters_safe_and_recovers(fsw):
    link, _ = fsw
    link.send(P.cmd_inject_fault(50.0))
    assert any(link.next_hk()["mode"] == 2 for _ in range(4)), "did not enter SAFE"
    link.send(P.cmd_exit_safe())
    assert link.next_hk()["mode"] == 2, "EXIT_SAFE must be rejected while hot"
    link.send(P.cmd_inject_fault(-50.0)); link.next_hk()
    link.send(P.cmd_exit_safe())
    assert any(link.next_hk()["mode"] == 1 for _ in range(3)), "did not recover to NOMINAL"