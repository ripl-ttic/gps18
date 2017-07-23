add_definitions(-std=gnu99)

add_executable(hr-gpsd
  gps.c
  main.c
  gps_display.c)

target_link_libraries (hr-gpsd
    pthread
    curses
    ${LCMTYPES_LIBS})

pods_use_pkg_config_packages(hr-gpsd
  lcm
  bot2-core)

pods_install_executables(hr-gpsd)




add_executable(hr-gps-lcm-listen
  gps_lcm_listen.c
  gps_display.c)

target_link_libraries (hr-gps-lcm-listen
    pthread
    curses
    ${LCMTYPES_LIBS})

pods_use_pkg_config_packages(hr-gps-lcm-listen
  lcm
  bot2-core)

pods_install_executables(hr-gps-lcm-listen)
