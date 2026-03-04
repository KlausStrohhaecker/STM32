cmake_minimum_required(VERSION 3.5...3.30)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#sudo apt install gcc-arm-none-eabi
#cmake -DCMAKE_TOOLCHAIN_FILE=/home/ksl/DevLocal/STM32-template-project/cmake/STM32-toolchain.cmake -DCMAKE_BUILD_TYPE=DEBUG /home/ksl/DevLocal/STM32-template-project

#set(MYFLAGS "-Wall -Wextra -Werror")
set(MYFLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${MYFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYFLAGS}")

project(STM32-test LANGUAGES C CXX ASM)

set(CMAKE_EXE_LINKER_FLAGS
    "-Wl,--gc-sections -static -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group -Wl,--print-memory-usage"
    CACHE INTERNAL "")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto")

file(GLOB LINKER_SCRIPTS "*.ld")
file(GLOB_RECURSE SOURCES_C "*.c" )
file(GLOB_RECURSE SOURCES_CXX "*.cpp" )
file(GLOB_RECURSE SOURCES_ASM "*.s" "*.S" )

set(ELF_NAME ${PROJECT_NAME}.elf)

add_executable(${ELF_NAME}
    ${SOURCES_C}
    ${SOURCES_CXX}
    ${SOURCES_ASM}
)

SET(INCLUDE_DIRS
    Core/Inc
    Drivers/CMSIS/Device/ST/STM32G4xx/Include
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    Drivers/STM32G4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/
    Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
    Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    USB_Device/App
    USB_Device/Target
    USB_DEVICE/App
    USB_DEVICE/Target)

target_compile_definitions(${ELF_NAME} PUBLIC -DUSE_HAL_DRIVER -DSTM32F446xx)
target_include_directories(${ELF_NAME} PUBLIC
    ${INCLUDE_DIRS}
)

target_link_options(${ELF_NAME} PUBLIC
    "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32F446RCTX_FLASH.ld"
#    "-Wl,-Map=/home/hhoegelo/dev/artifacts/builds/artefakt/bin/${PROJECT_NAME}.map"
)

add_custom_command(
  TARGET ${ELF_NAME}
  POST_BUILD
  COMMAND arm-none-eabi-objcopy -Obinary ${ELF_NAME} ${PROJECT_NAME}.bin
)

