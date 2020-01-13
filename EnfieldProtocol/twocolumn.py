#!/usr/bin/env python3
import curses
import time
from binascii import hexlify
import select

import serial


SHEIGHT = 1
TWIDTH = 16

PAUSE = False

ALLPKTS = []
MAXPKTS = int(1e6)

T0 = time.time()


def displaystream(dev, flname, stdscr, twin, dwin, owin, left):
    port = serial.Serial(dev, 115200, timeout=0.005)

    fl = open(flname, 'w+')

    tfmt = "%%%dd" % (TWIDTH - 1)

    def update():
        height, width = stdscr.getmaxyx()
        pkt = b''
        t = int(round((time.time() - T0) * 1000))
        c = port.read(1)
        while c:
            pkt += c
            c = port.read(1)
        if len(pkt):
            if not PAUSE:
                twin.scroll()
                dwin.scroll()
                owin.scroll()
                hpkt = hexlify(pkt)
                twin.addstr(height - SHEIGHT - 1, 0,
                            (tfmt % t)[:TWIDTH - 1])
                dwin.addstr(height - SHEIGHT - 1, 0, hpkt)
                fl.write((tfmt+"\t%s\n") % (t, hpkt.decode('utf-8')))
            global ALLPKTS
            ALLPKTS.append((t, pkt))
            ALLPKTS = ALLPKTS[-MAXPKTS:]
            twin.noutrefresh(0, 0, 0, 0, height - SHEIGHT - 1, TWIDTH)
            dwin.noutrefresh(0, 0, 0,
                             left, height - SHEIGHT - 1, width)
            return True
        return False

    return fl, port.fileno(), update


def resize_display(stdscr, twin=None, cwin=None, rwin=None, swin=None):
    height, width = stdscr.getmaxyx()
    if swin is None:
        swin = curses.newpad(SHEIGHT, width)
    else:
        swin.resize(SHEIGHT, width)
    swin.addstr(0, TWIDTH, "x")
    swin.addstr(0, (width + TWIDTH) // 2, "x")
    swin.noutrefresh(0, 0, height - SHEIGHT, 0, height, width)
    if twin is None:
        twin = curses.newpad(height - SHEIGHT, TWIDTH)
        twin.scrollok(True)
    twin.noutrefresh(0, 0, 0, 0, height - SHEIGHT, TWIDTH)
    if cwin is None:
        cwin = curses.newpad(height - SHEIGHT, (width - TWIDTH) // 2)
        cwin.scrollok(True)
    else:
        cwin.resize(height - SHEIGHT, (width - TWIDTH) // 2)
    cwin.noutrefresh(0, 0, 0, TWIDTH, height - SHEIGHT, (width + TWIDTH) // 2)
    if rwin is None:
        rwin = curses.newpad(height - SHEIGHT, (width - TWIDTH) // 2)
        rwin.scrollok(True)
    else:
        rwin.resize(height - SHEIGHT, (width - TWIDTH) // 2)
    rwin.noutrefresh(0, 0, 0, (width + TWIDTH) // 2, height - SHEIGHT, width)
    return twin, cwin, rwin, swin


def interface(stdscr, twin, cwin, rwin, swin):
    need_update = False
    height, width = stdscr.getmaxyx()
    ch = stdscr.getch()
    if ch == curses.KEY_RESIZE:
        resize_display(stdscr, twin, cwin, rwin, swin)
        need_update = True
    elif ch == ord('q'):
        raise SystemExit
    elif ch == ord('p'):
        global PAUSE
        PAUSE ^= True
        need_update = True
    elif PAUSE and ch == curses.KEY_UP:
        need_update = True
    elif PAUSE and ch == curses.KEY_DOWN:
        need_update = True
    if need_update:
        swin.clear()
        swin.addch(0, 0, ch)
        swin.noutrefresh(0, 0, height - SHEIGHT,
                         0, height, width)
    return need_update


def main(stdscr):
    stdscr.clear()
    stdscr.keypad(True)
    stdscr.timeout(0)
    command_dev = '/dev/ttyUSB1'
    command_file = 'command.txt'
    response_dev = '/dev/ttyUSB0'
    response_file = 'response.txt'
    twin, cwin, rwin, swin = resize_display(stdscr)
    curses.doupdate()
    poll = select.poll()
    updates = {}
    cfl, cf, cu = displaystream(command_dev, command_file, stdscr, twin, cwin,
                                rwin, TWIDTH)
    poll.register(cf, select.POLLIN)
    updates[cf] = cu
    rfl, rf, ru = displaystream(response_dev, response_file, stdscr, twin,
                                rwin, cwin, (curses.COLS + TWIDTH) // 2)
    poll.register(rf, select.POLLIN)
    updates[rf] = ru
    while True:
        need_update = False
        for fd, event in poll.poll(0.1):
            try:
                need_update |= updates[fd]()
            except curses.error:
                pass
            except SystemExit:
                cfl.flush()
                cfl.close()
                rfl.flush()
                rfl.close()
        try:
            need_update |= interface(stdscr, twin, cwin, rwin, swin)
        except curses.error:
            pass

        if need_update:
            curses.doupdate()


if __name__ == '__main__':
    curses.wrapper(main)
