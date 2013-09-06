
//****************************************************************************
// GloNav GPS Technology
// Copyright (C) 2001-2007 GloNav Ltd.
// March House, London Rd, Daventry, Northants, UK.
// All rights reserved
//
// Filename  GN_GPS_DataLogs.c
//
// $Header: $
// $Locker: $
//****************************************************************************


//****************************************************************************
//
// This is example code to illustrate how the Glonav GPS High-Level software
// can be integrated into the host platform. Note that, although this
// constitutes a fully-working GPS receiver, it is simplified relative to
// what would be expected in a real product. The emphasis is in trying to
// provide clarity of understanding of what is going on in the software, and
// how the various API function calls relate to each other, rather than in
// providing an efficient software implementation.
//
// The results from the GPS software are written to data log files. This is
// not what would be expected for a real system.
//
// The functions in this file constitute the data logging functionality.
//
// There are 4 log files:
//
//    1. The NMEA data log (NMEA_LOG).
//    2. The Measurement Engine (i.e. baseband chip) Debug data log (ME_LOG).
//    3. The Navigation Debug data log (NAV_LOG).
//    4. The Event Log (EVENT_LOG).
//
//****************************************************************************

#include "gps_api.h"
#include "GN_GPS_DataLogs.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define     NMEA_LOG_FILE   	"/data/GN_NMEA_Log.TXT"
#define     EVENT_LOG_FILE   	"/data/GN_Event_Log.TXT"
#define     ME_LOG_FILE   	"/data/GN_ME_Log.TXT"
#define     NAV_LOG_FILE   	"/data/GN_Nav_Log.TXT"
#define     TEST_LOG_FILE   	"/data/TEST_Log.TXT"

//*****************************************************************************
// Global data
static BL             g_Enable_Log[NUM_LOGS];   // Flag which logs are active
// update 2007.7.4
static int	hFile_Log[NUM_LOGS];

//*****************************************************************************
// Open the GPS log files
void GN_Open_Logs(void)
{
    int  i;

    memset(g_Enable_Log, 0, sizeof(g_Enable_Log));

    i = (int)NMEA_LOG;

    if(access(NMEA_LOG_FILE,F_OK) == 0){
    remove(NMEA_LOG_FILE);
    }

    hFile_Log[i] = open(NMEA_LOG_FILE, O_CREAT | O_RDWR);
    g_Enable_Log[i] = TRUE;

    i = (int)EVENT_LOG;
    if(access(EVENT_LOG_FILE,F_OK) == 0){
        remove(EVENT_LOG_FILE);
    }
    hFile_Log[i] = open(EVENT_LOG_FILE, O_CREAT | O_RDWR);
    g_Enable_Log[i] = TRUE;

#if 0
    i = (int)ME_LOG;
    if(access(ME_LOG_FILE,F_OK) == 0){
        remove(ME_LOG_FILE);
    }
    hFile_Log[i] = open(ME_LOG_FILE, O_CREAT | O_RDWR);
    //g_Enable_Log[i] = TRUE;
    g_Enable_Log[i] = FALSE;

    i = (int)NAV_LOG;
    if(access(NAV_LOG_FILE,F_OK) == 0){
        remove(NAV_LOG_FILE);
    }
    hFile_Log[i] = open(NAV_LOG_FILE, O_CREAT | O_RDWR);
    //g_Enable_Log[i] = TRUE;
    g_Enable_Log[i] = FALSE;

    i = (int)TEST_LOG;
    if(access(TEST_LOG_FILE,F_OK) == 0){
        remove(TEST_LOG_FILE);
    }
    hFile_Log[i] = open(TEST_LOG_FILE, O_CREAT | O_RDWR);
    //g_Enable_Log[i] = TRUE;
    g_Enable_Log[i] = FALSE;
#endif


    return;
}



//*****************************************************************************
// Close the GPS log files
void GN_Close_Logs(void)
{
    int i;

    // Close each of the log files
    for ( i = 0 ; i < NUM_LOGS ; i++ )
    {
        if ( g_Enable_Log[i] == TRUE )
        {
            close(hFile_Log[i]);
        }
    }
}


//*****************************************************************************
// Write data to the specified log file
// Write data to the specified log file
void Write_Data_To_Log(
       e_Data_Log  log,          // Data log type
       U2          num_bytes,    // Number of bytes to Write
       CH          *p_data )     // Pointer to the data
{
    unsigned int	WriteCnt;

    if(g_Enable_Log[log] == TRUE )
    {
        WriteCnt = write(hFile_Log[log], (void *) p_data, (unsigned int) num_bytes);
    }
}

//*****************************************************************************


