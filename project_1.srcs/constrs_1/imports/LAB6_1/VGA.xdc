################################################################################
#                                                                              #
#   vid_red                                                                    #
#                                                                              #
################################################################################
set_property PACKAGE_PIN V18 [get_ports {RGB[11]}]
set_property PACKAGE_PIN V19 [get_ports {RGB[10]}]
set_property PACKAGE_PIN U20 [get_ports {RGB[9]}]
set_property PACKAGE_PIN V20 [get_ports {RGB[8]}]

set_property IOSTANDARD LVCMOS33 [get_ports {RGB[11]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[10]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[9]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[8]}]


################################################################################
#                                                                              #
#   vid_green                                                                  #
#                                                                              #
################################################################################
set_property PACKAGE_PIN AA21 [get_ports {RGB[7]}]
set_property PACKAGE_PIN AB21 [get_ports {RGB[6]}]
set_property PACKAGE_PIN AA22 [get_ports {RGB[5]}]
set_property PACKAGE_PIN AB22 [get_ports {RGB[4]}]

set_property IOSTANDARD LVCMOS33 [get_ports {RGB[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[4]}]


################################################################################
#                                                                              #
#    vid_blue                                                                  #
#                                                                              #
################################################################################
set_property PACKAGE_PIN AB19 [get_ports {RGB[3]}]
set_property PACKAGE_PIN AB20 [get_ports {RGB[2]}]
set_property PACKAGE_PIN Y20 [get_ports {RGB[1]}]
set_property PACKAGE_PIN Y21 [get_ports {RGB[0]}]

set_property IOSTANDARD LVCMOS33 [get_ports {RGB[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RGB[0]}]


################################################################################
#                                                                              #
#    sync                                                                      #
#                                                                              #
################################################################################

set_property IOSTANDARD LVCMOS33 [get_ports vid_hsync]
set_property IOSTANDARD LVCMOS33 [get_ports vid_vsync]
set_property PACKAGE_PIN AA19 [get_ports vid_hsync]
set_property PACKAGE_PIN Y19 [get_ports vid_vsync]

set_property SLEW SLOW [get_ports {RGB[11]}]
set_property SLEW SLOW [get_ports {RGB[10]}]
set_property SLEW SLOW [get_ports {RGB[9]}]
set_property SLEW SLOW [get_ports {RGB[8]}]
set_property SLEW SLOW [get_ports {RGB[7]}]
set_property SLEW SLOW [get_ports {RGB[6]}]
set_property SLEW SLOW [get_ports {RGB[5]}]
set_property SLEW SLOW [get_ports {RGB[4]}]
set_property SLEW SLOW [get_ports {RGB[3]}]
set_property SLEW SLOW [get_ports {RGB[2]}]
set_property SLEW SLOW [get_ports {RGB[1]}]
set_property SLEW SLOW [get_ports {RGB[0]}]
