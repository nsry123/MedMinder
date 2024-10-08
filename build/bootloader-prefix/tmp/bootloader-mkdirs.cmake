# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/sunzeyuan/esp/v5.3/esp-idf_5.3/components/bootloader/subproject"
  "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader"
  "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader-prefix"
  "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader-prefix/tmp"
  "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader-prefix/src"
  "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/sunzeyuan/Downloads/DesktopScreenV4.0 2.3/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
