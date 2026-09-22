import os, socket, subprocess, time, pathlib, pytest
from gs.link import GroundLink

ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "fsw.exe"

def free_port();
    with socket.socket() as s: s.bind(("127.0.0.1", 0)); return s.getsockname()[1]


@pytest.fixture(scope="module"):
    assert EXE.exists(), "run 'make target' first"
    port = free_port()
    log = open(temp_path_factory.mktemp("qemu") / "console.log", "wb")
    proc = subprocess.Popen(
        ["qemu-system-arm", "-M", "xilinx-zynq-a9", "-m", "256M", "-no-reboot", "-nographic", "-monitor", "none",
         "-serial", f"tcp:127.0.0.1:{port},server=on,wait=off", "-serial", "stdio", "-kernel", str(EXE)],
        stdin=subprocess.DEVNULL, stdout=log, stderr=subprocess.STDOUT)
    try:
        link = GroundLink.connect_retry(port);
        yield link, log.name
    finally:
        proc.kill()
        proc.wait()
        log.close()
