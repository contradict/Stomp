import os.path as p
import os

DIR_OF_THIS_SCRIPT = p.abspath(p.dirname(__file__))

BASE_FLAGS = [
    "-Wall",
    "-Wextra",
    "-Werror",
    "-Wno-long-long",
    # '-Wno-variadic-macros',
    "-fexceptions",
    "-ferror-limit=10000",
    # '-DNDEBUG',
    "-std=c11",
    "-xc",
    "-D", "_XOPEN_SOURCE=500",
    "-D", "_DEFAULT_SOURCE",
    "-I", "/usr/local/include",
    "-I", "/usr/local/include/modbus",
    "-I", DIR_OF_THIS_SCRIPT + "/../LegBoard/Firmware/inc/export",
    "-I", DIR_OF_THIS_SCRIPT + "/Common",
    "-I", DIR_OF_THIS_SCRIPT + "/SimpleGait",
    "-I", DIR_OF_THIS_SCRIPT + "/HullControl/inc",
    "-I", DIR_OF_THIS_SCRIPT + "/HullControl/ringbuf/include",
]
if "ROS_DISTRO" in os.environ:
    ros_base = "/opt/ros/%(ROS_DISTRO)s" % os.environ
    BASE_FLAGS += ["-I", ros_base + "/include"]


SOURCE_EXTENSIONS = [
    ".cpp",
    ".cxx",
    ".cc",
    ".c",
]

HEADER_EXTENSIONS = [".h", ".hxx", ".hpp", ".hh"]


def Settings(**kwargs):
    return {
        "flags": BASE_FLAGS,
    }
