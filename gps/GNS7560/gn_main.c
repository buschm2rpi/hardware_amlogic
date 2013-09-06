
//*****************************************************************************
// GloNav GPS Technology
// Copyright (C) 2007 GloNav Ltd.
// March House, London Rd, Daventry, Northants, UK.
// All rights reserved
//
// Filename  GN_Port_Setup.c
//
// $Header: S:/GN_GPS_Nav_MKS/Arch/GNU/_GN_GPS/rcs/GN_Port_Setup.c 1.5 2008/02/29 09:41:09Z mohamed Rel mohamed $
// $Locker: mohamed $
//*****************************************************************************

#include "gps_api.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "GN_GPS_Example.h"

U4 gn_CLK_TCK  = 0;

//-----------------------------------------------------------------
// The following are parameters relating to uploading a patch file
// to the baseband chip
U1           gn_Patch_Status;       // Status of GPS baseband patch transmission
U2           gn_Cur_Mess[5];        // Current messages to send, of each 'stage'
U2		GPS_ROM_Version;
//-----------------------------------------------------------------


int GN_Port_Setup(
   CH *port,                     // i  - Port Name
   U4  baud,                     // i  - Port Baudrate
   CH *usage )                   // i  - Port Usage description
{
   // Return Argument Definition
   int            hPort;         // Handle to the opened Comm port
   // Local Data Definitions
   int            err;           // Error code
   struct stat    status;        // Returns the status of the baseband serial port.
   struct termios curr_term;     // The current serial port configuration descriptor.

   LOGGPS( "+GN_Port_Setup:\r\n" );

    RUNTO;
   hPort = -1;

   // Check for a valid PC COM Port Name
   if ( port[0] == '\0' )
   {
      LOGGPS( " GN_Port_Setup:  ERROR: %s Port %s  Invalid Port Name\r\n\r\n",
              (char*)usage, port );
      return( -1 );
   }

   // First check the port exists.
   // This avoids thread cancellation if the port doesn't exist.
   hPort = -1;
   if ( ( err = stat( port, &status ) ) == -1 )
   {
      LOGGPS( " GN_Port_Setup: stat(%s,*) = %d,  errno %d\r\n", port, err, errno );
      return( -1 );
   }

   // Open the serial port.
    hPort = open( port, (O_RDWR | O_NOCTTY | O_NONBLOCK) );
    //hPort = open( port, (O_RDWR | O_NOCTTY ) );
   if ( hPort <= 0 )
   {
      LOGGPS( " GN_Port_Setup: open(%s,*) = %d, errno %d\r\n", port, hPort, errno );
      return( -1 );
   }

   tcflush(hPort, TCIOFLUSH);
   // Get the current serial port terminal state.
   if ( ( err = tcgetattr( hPort, &curr_term ) ) != 0 )
   {
      LOGGPS( " GN_Port_Setup: tcgetattr(%d) = %d,  errno %d\r\n", hPort, err, errno );
      close( hPort );
      return( -1 );
   }

   // Set the terminal state to local (ie a local serial port, not a modem port),
   // raw (ie binary mode).
   //#ifdef __USE_BSD
      //curr_term.c_cflag  =  curr_term.c_cflag | CLOCAL;
      //cfmakeraw( &curr_term );
   //#else
      // Apparently the following does the same as cfmakeraw
      curr_term.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
      curr_term.c_oflag &= ~OPOST;
      curr_term.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
      curr_term.c_cflag &= ~(CSIZE|PARENB);
      curr_term.c_cflag |= CS8;
   //#endif
      curr_term.c_cflag &= ~CRTSCTS;

   // Disable modem hang-up on port close.
   //curr_term.c_cflag      =  curr_term.c_cflag & (~HUPCL);

   // Set way that the read() driver behaves.
// curr_term.c_cc[VMIN]   =  1;                 // read() returns when 1 byte available
   //curr_term.c_cc[VMIN]   =  0;                 // read() returns immediately
   //curr_term.c_cc[VTIME]  =  0;                 // read() Time-Out in 1/10th sec units

	tcsetattr(hPort, TCSANOW, &curr_term);
	tcflush(hPort, TCIOFLUSH);
	tcsetattr(hPort, TCSANOW, &curr_term);
	tcflush(hPort, TCIOFLUSH);
	tcflush(hPort, TCIOFLUSH);

   // Set the requested baud rate in the terminal state descriptor.
   switch( baud )
   {
      case 4800:
         baud = B4800;
         break;
      case 9600:
         baud = B9600;
         break;
      case 19200:
         baud = B19200;
         break;
      case 38400:
         baud = B38400;
         break;
      case 57600:
         baud = B57600;
         break;
      case 115200:
         baud = B115200;
         break;
      case 230400:
         baud = B230400;
         break;
      default:
         close( hPort );
         return( -1 );
         break;
   }

   // Set the input baud rates in the termios.
   if ( cfsetispeed( &curr_term, baud ) )
   {
      close( hPort );
      return( -1 );
   }

   // Set the output baud rates in the termios.
   if ( cfsetospeed( &curr_term, baud ) )
   {
      close( hPort );
      return( -1 );
   }

   // Set the Parity state to NO PARITY.
   //curr_term.c_cflag  =  curr_term.c_cflag  &  (~( PARENB | PARODD ));

   // Set the Flow control state to NONE.
   //curr_term.c_cflag  =  curr_term.c_cflag  &  (~CRTSCTS);
   //curr_term.c_iflag  =  curr_term.c_iflag  &  (~( IXON | IXOFF | IXANY ));

   // Set the number of data bits to 8.
   //curr_term.c_cflag  =  curr_term.c_cflag  &  (~CSIZE);
   //curr_term.c_cflag  =  curr_term.c_cflag  |  CS8;

   // Set 2 stop bits.
   //curr_term.c_cflag  =  curr_term.c_cflag  |  ( CSTOPB | CREAD );

//   LOGGPS( " curr_term   i = %08X   o = %08X   l = %08X   c = %08X   cc = %08X \r\n",
//           curr_term.c_iflag, curr_term.c_oflag, curr_term.c_lflag, curr_term.c_cflag, curr_term.c_cc );

   // Now set the serial port configuration and flush the port.
   //if ( tcsetattr( hPort, TCSAFLUSH, &curr_term ) != 0 )
   //{
      //close( hPort );
      //return( -1 );;
   //}

	tcsetattr(hPort, TCSANOW, &curr_term);


   return( hPort );
}

//*****************************************************************************
// Setup the GPS baseband configuration. This is optional.
//
void GPS_Configure( void )
{
   BL curr_config;
   BL set_config;
   s_GN_GPS_Config   Host_GN_GPS_Config;

   // Take a copy of the default GN GPS Configuration Settings.
   // Make the required changes and apply it back.
   curr_config = GN_GPS_Get_Config( &Host_GN_GPS_Config );
   if ( curr_config == TRUE )
   {
      //Host_GN_GPS_Config.FixInterval      = 2000;     // Fix at 1 sec intervals
      Host_GN_GPS_Config.FixInterval      = 1000;     // Fix at 1 sec intervals
      //Host_GN_GPS_Config.PosFiltMode = 3;
      Host_GN_GPS_Config.GPGLL_Rate = 0;   // Turn off NMEA $GPGLL
      Host_GN_GPS_Config.GPZDA_Rate = 0;  // Turn off NMEA $GPZDA
      Host_GN_GPS_Config.GPVTG_Rate = 0;  // Turn off NMEA $GPVTG

      set_config = GN_GPS_Set_Config( &Host_GN_GPS_Config );
      if ( set_config == FALSE )
      {
#if WRITE_LOG
         GN_GPS_Write_Event_Log(42,
                          (CH*)"Failed to set GPS configuration correctly\n");
#endif
      }
      else
      {
#if WRITE_LOG
         GN_GPS_Write_Event_Log(22,(CH*)"GPS configuration set\n");
#endif
      }
   }
   else
   {
#if WRITE_LOG
      GN_GPS_Write_Event_Log(37,(CH*)"Cannot get current GPS configuration\n");
#endif
   }

   return;
}

//#define UART_RX_BUFFER_SIZE       4096//4096     // Input buffer size
//#define UART_TX_BUFFER_SIZE      2048// 2048     // Output buffer size
//static U1 *buffer;
//static U1 buffer[UART_RX_BUFFER_SIZE];
//s_CircBuff  UARTrx_cb;

void GPS_INIT(void)
{
    RUNTO;

    //buffer = (U1*)malloc(sizeof(U1) * UART_RX_BUFFER_SIZE);
    //if (buffer == NULL)
	//	    LOGD("malloc fail");
    //UARTrx_cb.start_buf = &buffer[0];
    //UARTrx_cb.read = UARTrx_cb.start_buf;
    //UARTrx_cb.write = UARTrx_cb.start_buf;
    //UARTrx_cb.end_buf = UARTrx_cb.start_buf+ UART_RX_BUFFER_SIZE;


    gn_Patch_Status = 0;
    gn_Cur_Mess[0]  = 0;
    gn_Cur_Mess[1]  = 0;
    gn_Cur_Mess[2]  = 0;
    gn_Cur_Mess[3]  = 0;
    gn_Cur_Mess[4]  = 0;
    gn_CLK_TCK = (U4)sysconf(_SC_CLK_TCK);
    LOGGPS("GPS_INT:gn_CLK_TCK = %d\n",gn_CLK_TCK);

#if WRITE_LOG
    // Open the Data Log files
    GN_Open_Logs();
#endif
    // Initialise the GPS software. This API function MUST be called.
    GN_GPS_Initialise();

    GPS_Configure();
}



