
readme.txt for ADSP-BF518F EZ-Board Precision Time Protocol (PTP)

Last updated:	March 10, 2009

Test history:
	- Last tested March 10, 2009
	- VisualDSP 5.0 Update 6
	- ADSP-BF518F EZ-Board Lite Rev 0.2

__________________________________________________________

CONTENTS

I.    FUNCTIONAL DESCRIPTION
II.   NETWORK SETUP
III.  IMPLEMENTATION DESCRIPTION
IV.   OPERATION DESCRIPTION
V.    MISCELLANEOUS


__________________________________________________________

I.  FUNCTIONAL DESCRIPTION

This directory contains example Precision Time Protocol (PTP) projects,
which run on an ADSP-BF518F EZ-Board and on Windows.  By following the 
instructions below, you can run the Windows PTP application, win32-ptp.exe, 
as master clock, and the ADSP-BF518 PTP application as slave clock. 
These two clocks can synchronize through Ethernet.

II. NETWORK SETUP



      ----------------           ----------              -------------------
      |              |    1      |        |      1       |                 |
      |      PC      |-----------| Router |--------------| BF518F EZ-Board |
      |              |           |        |              |                 |
      ----------------           ----------              -------------------
              |                                                    |
              ------------------------------------------------------
			        2

There are 2 network connections can be used to run PTP example.

1. Connect both PC and BF518F EZ-Board to a router
2. Connect PC and EZ-Board directly with ethernet cable. Since ethernet device driver 
enables auto MDI-X in PHY controller, crossover ethernet cable is not necessary.

The communication of PTP example is based on UDP multicast that may be blocked by 
router and firewall software. As the result, the master clock (PC) and slave clock(BF518F) 
cannot synchronize each other successfully, and both clocks will run independently. Therefore 
multicast filter in router and PC firewall software must be turned off.

When connection 2 is used, you do not have to set up static IP address for PC and Blackfin. Since 
both PC and lwIP support Auto IP, they will generate auto IP address, 169.254.0.0 - 169.254.255.255
automatically without DHCP involved.

There will be adverse impact on the performance of PTP clock synchronization when use connection 1 
because of the delay introduced by router.



__________________________________________________________

II.  IMPLEMENTATION DESCRIPTION

This application consists of a Windows console-based PTP program 
and a Blackfin PTP program. 

Windows program:    <install_path>\Blackfin\Examples\ADSP-BF518F EZ-Board\LAN\PTP\win32\win32-ptp.exe
Blackfin projects:  <install_path>\Blackfin\Examples\ADSP-BF518F EZ-Board\LAN\PTP\BF518

Executes win32-ptp.exe on Windows first, and then load ptpd.dxe on the EZ-Board.  


__________________________________________________________

IV.  OPERATION DESCRIPTION

This Blackfin PTP applicaton displays clock messages over the UART connector.  
By placing a serial cable between the J2 connector and a COM port on your PC,
you can receive clock status information.  In order to receive clock 
information, run a terminal application on the PC with these settings:
*  115200 baud
*  8 data bits 
*  no parity
*  1 stop bit
*  no flow control

1. Open <install_path>\Blackfin\Examples\ADSP-BF518F EZ-Board\LAN\PTP\BF518\ptp.dpj project
2. Build and load the .dxe file.
3. On Windows, open a console window and run win32-ptp.exe 
   <install_path>\Blackfin\Examples\ADSP-BF518F EZ-Board\LAN\PTP\win32\win32-ptp.exe
4. Run the .dxe file at the Blackfin end. 
5. Upon success, PTP packets are transmitted between the Blackfin processor 
   and Windows PTP applications,
   Both applications print clock status.
   
If both master and slave clock synchronize properly, the slave clock (BF518F) should be < 1s, such as
(ptpd debug) fromInternalTime:         0s    62792160ns ->         0s    62792160ns

If the slave clock(BF518F) > 1s and keep incrementing, it means the clock synchronization between 
master and slave fails. Please check the network connection, router or firewall settings.


More information regarding PTPd can be found on the web at:  http://ptpd.sourceforge.net/doc.html
More information regarding lwIP is provided in the <install_path>\Blackfin\lib\src\lwip\contrib\ports\ADSP-Blackfin\docs\lwip_userguide.doc.   


__________________________________________________________

V. Miscellaneous   

1. Firewall running on windows system my wrongly interfere with the PTP software. You need to
   disable firewall for any potential packet loss or delays.

2. If routers, switchs or hubs are present between PTP slave and PTP master then it may introduce
   significant delays.

