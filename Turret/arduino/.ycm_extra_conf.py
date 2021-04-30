import os
import os.path
import logging
import ycm_core

try:
    ARDUINO = os.environ['ARDUINO']
except KeyError:
    ARDUINO = "%s/Arudino" % os.environ['HOME']

BASE_FLAGS = [
    '-Wall',
    '-Wextra',
    '-Werror',
    '-ferror-limit=10000',
    '-ffunction-sections',
    '-fdata-sections',
    '-fno-threadsafe-statics',
    '-fpermissive',
    '-fno-exceptions',
    '-fno-threadsafe-statics',
    '-flto',
    '-fno-devirtualize',
    '-std=c++11',
    '-xc++',
    '-I', ARDUINO + '/hardware/arduino/avr/cores/arduino',
    '-I', ARDUINO + '/hardware/arduino/avr/cores/arduino/api',
    '-I', ARDUINO + '/hardware/arduino/avr/variants/mega',
    '-I', ARDUINO + '/hardware/tools/avr/avr/include',
    '-I', 'arduino/libraries/I2C',
    '-I', 'arduino/libraries/MPU6050',
    '-D', '__AVR_ATmega2560__',
    '-D', 'F_CPU=16000000L',
    '-D', 'ARDUINO=188',
    '-D', 'ARDUINO_ARCH_AVR',
    '-D', '__PROG_TYPES_COMPAT__',
    '-D', 'SERIAL_TX_BUFFER_SIZE=256',
    '-D', 'SERIAL_RX_BUFFER_SIZE=256',
]

SOURCE_EXTENSIONS = [
    '.cpp',
    '.cxx',
    '.cc',
    '.c',
]

HEADER_EXTENSIONS = [
    '.h',
    '.hxx',
    '.hpp',
    '.hh'
]


def IsHeaderFile(filename):
    extension = os.path.splitext(filename)[1]
    return extension in HEADER_EXTENSIONS


# cmake can generate the database, add
# set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
def GetCompilationInfoForFile(database, filename):
    if IsHeaderFile(filename):
        basename = os.path.splitext(filename)[0]
        for extension in SOURCE_EXTENSIONS:
            replacement_file = basename + extension
            if os.path.exists(replacement_file):
                compilation_info = database.GetCompilationInfoForFile(replacement_file)
                if compilation_info.compiler_flags_:
                    return compilation_info
        return None
    return database.GetCompilationInfoForFile(filename)


def FindNearest(path, target):
    candidate = os.path.join(path, target)
    if(os.path.isfile(candidate) or os.path.isdir(candidate)):
        logging.info("Found nearest " + target + " at " + candidate)
        return candidate
    else:
        parent = os.path.dirname(os.path.abspath(path))
        if(parent == path):
            raise RuntimeError("Could not find " + target)
        return FindNearest(parent, target)


def MakeRelativePathsInFlagsAbsolute(flags, working_directory):
    if not working_directory:
        return list(flags)
    new_flags = []
    make_next_absolute = False
    path_flags = ['-isystem', '-I', '-iquote', '--sysroot=']
    for flag in flags:
        new_flag = flag

        if make_next_absolute:
            make_next_absolute = False
            if not flag.startswith('/'):
                new_flag = os.path.join(working_directory, flag)

        for path_flag in path_flags:
            if flag == path_flag:
                make_next_absolute = True
                break

            if flag.startswith(path_flag):
                path = flag[len(path_flag):]
                new_flag = path_flag + os.path.join(working_directory, path)
                break

        if new_flag:
            new_flags.append(new_flag)


def FlagsForInclude(root):
    try:
        include_path = FindNearest(root, 'include')
        flags = []
        for dirroot, dirnames, filenames in os.walk(include_path):
            for dir_path in dirnames:
                real_path = os.path.join(dirroot, dir_path)
                flags = flags + ["-I" + real_path]
        return flags
    except Exception:
        return None


def FlagsForCompilationDatabase(root, filename):
    try:
        compilation_db_path = FindNearest(root, 'compile_commands.json')
        compilation_db_dir = os.path.dirname(compilation_db_path)
        logging.info("Set compilation database directory to " + compilation_db_dir)
        compilation_db = ycm_core.CompilationDatabase(compilation_db_dir)
        if not compilation_db:
            logging.info("Compilation database file found but unable to load")
            return None
        compilation_info = GetCompilationInfoForFile(compilation_db, filename)
        if not compilation_info:
            logging.info("No compilation info for "
                         + filename + " in compilation database")
            return None
        return MakeRelativePathsInFlagsAbsolute(
            compilation_info.compiler_flags_,
            compilation_info.compiler_working_dir_)
    except Exception:
        return None


def FlagsForFile(filename):
    root = os.path.realpath(filename)
    compilation_db_flags = FlagsForCompilationDatabase(root, filename)
    if compilation_db_flags:
        final_flags = compilation_db_flags
    else:
        final_flags = BASE_FLAGS
        include_flags = FlagsForInclude(root)
        if include_flags:
            final_flags = final_flags + include_flags
    return {
        'flags': final_flags,
        'do_cache': True
    }
