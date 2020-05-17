import os.path as p

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
    "-std=gnu17",
    "-xc",
    "-mthumb",
    "-DSTM32F722xx",
    "-mmcu=cortex-m7",
    "-mthumb",
    "-mfpu=fpv5-sp-d16",
    "-mfloat-abi=hard",
    "--sysroot",
    "/usr/lib/gcc/arm-none-eabi",
    "-isystem",
    "/usr/include/newlib/",
    "-isystem",
    "/usr/lib/gcc/arm-none-eabi/6.3.1/include",
    "-D", "ChompLegBoard",
    "-I",
    p.join(DIR_OF_THIS_SCRIPT, "Drivers/CMSIS/Core/Include"),
    "-I",
    p.join(DIR_OF_THIS_SCRIPT, "Drivers/CMSIS/Device/ST/STM32F7xx/Include"),
    "-I",
    p.join(DIR_OF_THIS_SCRIPT, "Drivers/CMSIS/DSP/Include"),
    "-I",
    p.join(DIR_OF_THIS_SCRIPT, "Drivers/CMSIS/NN/Include"),
    "-I",
    p.join(DIR_OF_THIS_SCRIPT, "Drivers/STM32F7xx_HAL_Driver/Inc"),
    "-I",
    p.join(
        DIR_OF_THIS_SCRIPT,
        "FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/"),
    "-I",
    p.join(DIR_OF_THIS_SCRIPT,
           "FreeRTOS/Source/include/"),
    "-I", p.join(DIR_OF_THIS_SCRIPT, "inc"),
    "-I", p.join(DIR_OF_THIS_SCRIPT, "Drivers/BSP/ChompLegBoard"),
    "-I", p.join(DIR_OF_THIS_SCRIPT, "Drivers/BSP/STM32F7xx_Nucleo_144"),
    "-I", p.join(DIR_OF_THIS_SCRIPT, "Drivers/BSP/Components/ads57x4"),
    "-I", p.join(DIR_OF_THIS_SCRIPT, "Drivers/BSP/Components/is31fl3235"),
]

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
