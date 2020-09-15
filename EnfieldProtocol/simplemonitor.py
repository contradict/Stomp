import serial
import sys
from binascii import hexlify
import threading


def monitor(portname, outfile=sys.stdout, stop_evt=None):
    data = b""
    extra = b""
    with serial.Serial(portname, 115200, timeout=0.01) as sp:
        while True if stop_evt is None else not stop_evt.wait(0):
            data += sp.read()
            while len(data) > 8:
                if(data[0] == 0x24 and
                   data[1] == 0x43 and
                   data[5] == 0x23):
                    if len(extra):
                        outfile.write("extra: %s " % hexlify(extra))
                        extra = b""
                    outfile.write(
                        "cmd: 0x%02x = 0x%04x\n" % (data[2], (data[3] << 8) + data[4]))
                    data = data[8:]
                elif (data[0] == 0x2b and
                      data[3] == 0x23):
                    if len(extra):
                        outfile.write("extra: %s" % hexlify(extra))
                        extra = b""
                    outfile.write("rsp: 0x%04x\n" % ((data[1] << 8) + data[2]))
                    data = data[6:]
                else:
                    extra += data[0:1]
                    data = data[1:]


def monitorfile(portname, filename, stop):
    with open(filename, "w") as f:
        monitor(portname, f, stop)


def monitorall(portnames=["/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2"],
               filenames=["curl.txt", "lift.txt", "swing.txt"]):
    ts = []
    stop = threading.Event()
    for pn, fn in zip(portnames, filenames):
        ts.append(threading.Thread(target=monitorfile, args=(pn, fn, stop)))
    for t in ts:
        t.start()
    def stopfn():
        stop.set()
        for t in ts:
            t.join()

    return stopfn
