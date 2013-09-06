
//****************************************************************************
// GPS IP Centre, ST-Ericsson (UK) Ltd.
// Copyright (c) 2009 ST-Ericsson (UK) Ltd.
// 15-16 Cottesbrooke Park, Heartlands Business Park, Daventry, NN11 8YL, UK.
// All rights reserved.
//
// Filename gps_ptypes.h
//
// $Header: S:/GN_GPS_Nav_MKS/Arch/GNU/_GN_GPS/rcs/gps_ptypes.h 1.5 2009/04/23 15:31:25Z geraintf Exp $
// $Locker: $
//****************************************************************************
//
// GPS platform primitive typedefs


#ifndef GPS_API_H
#define GPS_API_H

// Include all the header files required for the GN_GPS_Task modules here.

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gps_ptypes.h"
#include "GN_GPS_api.h"
#include "GN_GPS_DataLogs.h"

#include <cutils/log.h>
#include <hardware/gps.h>

#define LOG_GPS
//#undef LOG_GPS

#ifdef LOG_GPS
#define RUNTO  LOGD("%s:  %d  called ", __FUNCTION__, __LINE__)
#define LOGGPS(...)     LOGD(__VA_ARGS__)
#define WRITE_LOG 1
#else
#define RUNTO            ((void)0)
#define LOGGPS(...)     ((void)0)
#define WRITE_LOG 0
#endif

//***************************************************

extern U1  gn_Patch_Status;               // Status of GPS baseband patch transmission
//extern U1  gn_Patch_Progress;             // % progress of patch upload for each stage

extern U4 gn_CLK_TCK ;
//extern int g_utc_diff;
extern long long g_utc_diff;

#define GN_GPS_GNB_PATCH_510     // Include patch for GNS7560 ROM5 v510

typedef struct {
    int                     init;
    int                     start;
    int                     fd;
    GpsCallbacks            callbacks;
    pthread_t               thread;
    pthread_t               task_thread;
    int                     control[2];
    int                     freq;

} GpsState;

extern GpsState  g_gps_state[1];

// Setup a Linux / POSIX comm Port.
int GN_Port_Setup(
   CH *port,                  // i  - Port Name
   U4  baud,                  // i  - Port Baudrate
   CH *useage );              // i  - Port Usage description

void GPS_INIT(void);


// This function is called from the GN GPS Library 'callback' function
// GN_GPS_Write_GNB_Patch() to start / set-up the GN Baseband Patch upload.
U2 GN_Setup_GNB_Patch(
   U2 ROM_version,            // i  - GN Baseband Chips ROM SW Version
   U2 Patch_CkSum );          // i  - GN Baseband Chips ROM SW Checksum


// Upload the next set of patch code to the GloNav GPS baseband.
// This function sends up to a maximum of Max_Num_Patch_Mess sentences each time
// it is called.
// The complete set of patch data is divided into six stages.
void GN_Upload_GNB_Patch(
   U4 Max_Num_Patch_Mess );   // i  - Maximum Number of Patch Messages to Upload


#endif   // GPS_API_H

// end of file
//char UART_Input_Task(void);
