import struct
import time
import random
from binascii import hexlify

import crcmod

# Known registers are named, addresss that return a value are present but
# commented out. Other addresses time out.
READ_REGISTERS = {
    "ProportionalGain": 112,
    "DerivativeGain": 113,
    # "": 114,
    # "": 115,
    # "": 116,
    # "": 117,
    # "": 118,
    "ForceDamping": 119,
    # "": 120,
    # "": 121,
    # "": 122,
    # "": 123,
    # "": 124,
    # "": 125,
    "Offset": 126,
    "CommandInput": 127,
    "FeedbackInput": 128,
    "AreaRatio": 129,
    "CylinderBore": 130,
    "MinimumPosition": 131,
    "MaximumPosition": 132,
    "PortConnection": 133,
    "RampUp": 134,
    "RampDown": 135,
    # "": 136,
    "DeadBand": 137,
    "DitherAmplitude": 138,
    "ValveOffset": 139,
    # "": 140,
    # "": 141,
    # "": 142,
    # "": 143,
    "FeedbackPolarity": 144,
    "DigitalCommand": 145,
    "CommandSource": 146,
    # "": 147,
    # "": 148,
    # "": 149,
    # "": 150,
    # "": 151,
    "AnalogCommand": 152,
    "FeedbackPosition": 153,
    "BaseEndPressure": 154,
    "RodEndPressure": 155,
    # "": 156,
    # "": 157,
    "SpoolPosition": 158,
    # "": 159,
    # "": 160,
    # "": 161,
    # "": 162,
    # "": 163,
    # "": 164,
    # "": 165,
    # "": 166,
    # "": 167,
    # "": 168,
    # "": 169,
    # "": 170,
    # "": 171,
    # "": 172,
    # "": 173,
    # "": 174,
    # "": 175,
    # "": 176,
    # "": 178,
    # "": 179,
    # "": 180,
    # "": 181,
    # "": 182,
    # "": 183,
    # "": 184,
}

# All values the Enfield tool writes. Other addresses may be writeable, but I
# have no way to check.
WRITE_REGISTERS = {
    "ProportionalGain": 1,   # 0-1000
    "DerivativeGain": 2,     # 0-1000
    "ForceDamping": 8,       # 0-1000
    "AreaRatio": 12,         # 32767 * (rod end piston area / bore area)
    "CylinderBore": 13,      # 1024 * Cylinder Bore in inches
    "Offset": 15,            # 0-1000
    "CommandInput": 16,      # 0-4-20mA, 1-0-10v
    "FeedbackInput": 17,     # 0-4-20mA, 1-0-10v
    "MinimumPosition": 20,   # 0-1000
    "MaximumPosition": 21,   # 0-1000
    "PortConnection": 22,    # 0-standard, 1-transposed
    "RampUp": 23,            # 0-1000
    "RampDown": 24,          # 0-1000
    "DeadBand": 26,          # 0-1000
    "DitherAmplitude": 27,   # 0-1000
    "ValveOffset": 28,       # 0-1000
    "FeedbackPolarity": 33,  # 0-normal, 1-inverted
    "CommandSource": 89,     # 1-analog, 0-slider
    "DigitalCommand": 88,    # 0-4095
    # "": 149   # written with value 0x2222 on startup by the Enfield tool
    # "": 177,  # written to 0 on Enfield tool startup
    # "": 185,  # written to 0 on Enfield tool startup
    # "": 224,  # always written 0 after changing command input type
    "SaveConfiguration": 225,  # write 0 to save configuration
}

COMMAND_LEN = 8
RESPONSE_LEN = 6

crcfun = crcmod.predefined.mkPredefinedCrcFun('modbus')


def create_command(register, value=0x1111):
    """ Create a command packet

    register - Byte value at start of packet
    vlaue - optional payload
    """
    pkt = b'$C' + struct.pack('xBH', register, value)[1:] + b'#'
    crc = crcfun(pkt)
    pkt += struct.pack('H', crc)
    return pkt


class CrcError(Exception):
    """ CRC Cechk failed on received packet"""
    pass


class PacketLengthError(Exception):
    """ Packet length was not as expected"""
    pass


class PacketFormatError(Exception):
    """ Packet was missing expected fixed values"""
    pass


class TimeoutException(Exception):
    """ No data received in specified interval"""
    pass


def parse_response(data):
    """ Parse the response from the S2 serial device

    data - bytes read fro mthe serial port

    returns (value, remaining_data)

    If nothing can be parsed, value is None. Keep appending received bytes to
    remaining_data and calling again.
    """
    if data[:1] != b'+':
        data = data[1:]
        return None, data
    if len(data) < 4:
        return None, data
    if data[3:4] != b'#':
        return None, data[1:]
    if crcfun(data[:RESPONSE_LEN]):
        return None, data[1:]
    value, = struct.unpack('H', data[1:3])
    return value, data[RESPONSE_LEN:]


def parse_command(data):
    """ Parse a command value sent to the S2 serial device

    Used with a wiretap setup while monitoring the Enfield tool. Works the same
    way as parse_response, keep appending data until a value is parsed
    """
    if len(data) < 2:
        return None, data
    if data[:2] != b'$C':
        return None, data[1:]
    if len(data) < 6:
        return None, data
    if data[5:6] != b'#':
        return None, data[1:]
    if crcfun(data[:COMMAND_LEN]):
        return None, data[1:]
    register, value = struct.unpack('xBH', data[1:5])
    return (register, value), data[COMMAND_LEN:]


def transfer(port, register, data=0x1111, timeout=0.2):
    """ Send a transfer to an S2 device connected to port

    register - address on S2 device
    data - optional, always 0x1111 during reads by Enfield software.

    Returns data value parsed from response packet.
    """
    cmd = create_command(register, data)
    port.write(cmd)
    value = None
    now = time.time()
    data = port.read(RESPONSE_LEN)
    while value is None:
        if time.time() - now > timeout:
            raise TimeoutException()
        data += port.read(len(data) - RESPONSE_LEN)
        value, data = parse_response(data)
    return value


def monitor(cmdport, respport, ignore=[]):
    """ Monitor communication between Enfield tool and S2 device

    cmdport - receive wire of this serial device is connected to data fom
    computer to S2

    respport - receive wire of this serial device is connected to data from S2
    to computer
    """
    cmdport.flushInput()
    respport.flushInput()

    while True:
        t0 = time.time()
        cmd_data = cmdport.read(COMMAND_LEN)
        cmd_pkt, cmd_data = parse_command(cmd_data)
        while cmd_pkt is None:
            cmd_data += cmdport.read(COMMAND_LEN)
            cmd_pkt, cmd_data = parse_command(cmd_data)
        register, wvalue = cmd_pkt

        resp_data = respport.read(RESPONSE_LEN)
        value, resp_data = parse_response(resp_data)
        while value is None:
            resp_data += respport.read(RESPONSE_LEN)
            value, resp_data = parse_response(resp_data)
        t1 = time.time()
        if register in ignore:
            continue
        print("%15d %2x %4x %4x %15d" % (
              int(t0 * 1000), register, wvalue, value, int(1000 * (t1 - t0))))


def parseint(w):
    if w[:2] == '0x':
        return int(w, 16)
    else:
        return int(w)


def interact(enfport, unknown="ignore"):
    """ Interactive probing of S2 device

    enfport - serial port connected to S2 device.
    """
    enfport.flush()
    enfport.flushInput()

    read_names = {v: k for (k, v) in READ_REGISTERS.items()}
    while True:
        action = input()
        if len(action):
            words = action.split(' ')
            if words[0] == 't' and len(words) > 1:
                try:
                    if words[1] in WRITE_REGISTERS:
                        register = WRITE_REGISTERS[words[1]]
                    elif words[1] in READ_REGISTERS:
                        register = READ_REGISTERS[words[1]]
                    else:
                        register = parseint(words[1])
                    data = 0x1111
                    if len(words) == 3:
                        data = parseint(words[2])
                except Exception:
                    print("Nope")
                    continue
                v = transfer(enfport, register, data)
                print("v=%d" % v)
            elif words[0] == 'u' and len(words) > 1:
                unknown = words[1]
            elif words[0] == 'q':
                break
        for r in range(112, 186):
            n = read_names.get(r)
            if unknown == 'ignore' and n is None:
                continue
            elif unknown == 'only' and n is not None:
                continue
            x = transfer(enfport, r)
            print("%3d %5d %s" % (r, x, n))


def corrupttest(port):
    sequence = []
    ping = create_command(0x91, 0x00a3)
    while True:
        ncorrupt = random.randint(1, 5)
        for _ in range(ncorrupt):
            cmd = create_command(0x91, 0x00a3)
            corrupt_idx = random.choice([0, 1, 5, 6, 7])
            corrupt_value = random.choice(
                [x for x in range(255) if x not in [0x24, 0x43, 0x2b, 0x23]])
            cmd = cmd[:corrupt_idx] + bytes([corrupt_value]) + cmd[corrupt_idx + 1:]
            if random.choice([True, False]):
                crc = crcfun(cmd)
                cmd = cmd[:-2] + struct.pack('H', crc)
            sequence.append(cmd)
            port.write(cmd)
            print(hexlify(cmd), hexlify(port.read(6)))
        for _ in range(5):
            port.write(ping)
            rsp = port.read(6)
            if len(rsp) == 6:
                print("ping success")
                break
            else:
                print("ping fail")
        else:
            break
    return sequence
