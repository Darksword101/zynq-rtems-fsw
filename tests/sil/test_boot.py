def test_console_banner(fsw):
    link, logfile = fsw
    link.next_hk()
    text = open(logfile, "rb").read().decode(errors="replace")
    assert "FSW READY" in text
    assert "FATAL" not in text and "BLOWN STACK" not in text
