add_definitions(-std=gnu99)

add_executable(gpsd
  gps.c
  main.c
  gps_display.c)

target_link_libraries (gpsd
    pthread
    curses
    ${LCMTYPES_LIBS})

pods_use_pkg_config_packages(gpsd
  lcm
  bot2-core)

pods_install_executables(gpsd)




add_executable(gps-lcm-listen
  gps_lcm_listen.c
  gps_display.c)

target_link_libraries (gps-lcm-listen
    pthread
    curses
    ${LCMTYPES_LIBS})

pods_use_pkg_config_packages(gps-lcm-listen
  lcm
  bot2-core)

pods_install_executables(gps-lcm-listen)
