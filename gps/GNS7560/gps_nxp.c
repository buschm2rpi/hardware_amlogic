
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>

#define  LOG_TAG  "gps_gns"
#include <cutils/log.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <hardware/gps.h>

#include "gps_api.h"

/* the name of the qemud-controlled socket */

#define MAX_CMD_TRY_COUNT 32
#define GNS_DEVICES     "/dev/ttyS10"


//#define LOG_GPS
#undef LOG_GPS
#ifdef LOG_GPS
#  define  D(...)   LOGD(__VA_ARGS__)
#define  GPS_DEBUG  1
#else
#  define  D(...)   ((void)0)
#define  GPS_DEBUG  0
#endif

static void gps_assertion_failed(const char *files, int lines, const char * cond, int ext)
{
    LOGGPS("%s:%d  %s", files, lines, cond);
}
#define ENSURE(cond)  if(cond) gps_assertion_failed(__FILE__, __LINE__, #cond, 0);


#define  FIX_TIME 1

#if FIX_TIME
static const char time_file[] = "/data/gps_fix_time.txt" ;
int file_fd = -1 ;
#endif


/* commands sent to the gps thread */
enum {
    CMD_QUIT = 0,
    CMD_START,
    CMD_STOP,
    CMD_FREQ,
    CMD_CLEAR_AID
};

//TODO: support more than one gps observer.
GpsState  g_gps_state[1];
GpsCallbacks g_save_cbs;
GpsLocation  g_fix;
GpsStatus     g_status ;
GpsSvStatus  g_svStatus ;
//int g_utc_diff = 0;
long long g_utc_diff = 0;
s_GN_GPS_Nav_Data  g_nav_data;       // The latest Nav solution

int g_Task_Exit;

//extern U1           gn_Patch_Status;       // Status of GPS baseband patch transmission

//extern int GN_Port_Setup( CH *port, U4  baud, CH *usage );
//extern void GPS_INIT(void);

static int gns_gps_stop();

#if FIX_TIME
int writeTimeForFixBegin()
{
    int count ;
    char str[128];
    struct timeval tv;              // Local OS Real-Time-Clock Time in timeval format

    if(access(time_file, F_OK) == 0) {
        remove(time_file);
    }

    file_fd = open(time_file, O_CREAT | O_RDWR);
    if(file_fd < 0){
        return -1 ;
    }
    if(gettimeofday(&tv, NULL) < 0)
    {
        LOGGPS( "writeTimeForFixBegin: gettimeofday() failed" );
        return -2 ;
    }

    count = sprintf(str,"writeTimeForFixBegin: second = %d , msecond = %d \n",
                    (int)(tv.tv_sec%10000), (int)(tv.tv_usec/1000));

    write(file_fd, (void *)str, count);
    return 0;
}

int writeTimeForFixEnd()
{
    int count ;
    char str[128];
    struct timeval tv;              // Local OS Real-Time-Clock Time in timeval format

    if(file_fd < 0) {
        return -1 ;
    }

    if(gettimeofday( &tv, NULL) < 0)
    {
        LOGGPS( "writeTimeForFixEnd: gettimeofday() failed" );
        return -2 ;
    }

    count = sprintf(str,"writeTimeForFixEnd: second = %d , msecond = %d \n",
                    (int) (tv.tv_sec % 10000), (int) (tv.tv_usec/1000));

    write(file_fd, (void*)str, count);
    close(file_fd);
    file_fd = -1;
    return 0;
}
#endif

static void GN_GPS_message(void)
{
    RUNTO;
    time_t     fix_time;
    struct tm  fix_tm;
    char reciver_status;

    //reciver_status = UART_Input_Task();
    //if (reciver_status == TRUE)
    {

    memset(&g_nav_data,0,sizeof(g_nav_data));

    GN_GPS_Update();

    if(GN_GPS_Get_Nav_Data(&g_nav_data) == TRUE) {
        //if((g_nav_data.Valid_3D_Fix == TRUE) || (g_nav_data.Valid_2D_Fix == TRUE)) {
            memset(&g_fix, 0, sizeof(g_fix));

            if(g_nav_data.Longitude > 0 || g_nav_data.Latitude > 0) {
#if FIX_TIME
            writeTimeForFixEnd();
#endif
                g_fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
                g_fix.latitude  = g_nav_data.Latitude;
                g_fix.longitude = g_nav_data.Longitude;
            }

            if(g_nav_data.Valid_3D_Fix == TRUE) {
                g_fix.flags    |= GPS_LOCATION_HAS_ALTITUDE;
                g_fix.altitude = g_nav_data.Altitude_MSL;
            }

            if(g_nav_data.SpeedOverGround > 0.0) {
                g_fix.flags  |= GPS_LOCATION_HAS_SPEED;
                g_fix.speed = g_nav_data.SpeedOverGround;
            }

            if(g_nav_data.CourseOverGround > 0.0) {
                g_fix.flags |= GPS_LOCATION_HAS_BEARING;
                g_fix.bearing = g_nav_data.CourseOverGround;
            }

            if(g_nav_data.P_DOP > 0.0 && g_nav_data.P_DOP != 99.99) {
                g_fix.flags |= GPS_LOCATION_HAS_ACCURACY;
                g_fix.accuracy = g_nav_data.P_DOP;
            }

            fix_tm.tm_hour = g_nav_data.Hours;
            fix_tm.tm_min  = g_nav_data.Minutes;
            fix_tm.tm_sec  = g_nav_data.Seconds;
            fix_tm.tm_year = g_nav_data.Year - 1900;
            fix_tm.tm_mon  = g_nav_data.Month - 1;
            fix_tm.tm_mday = g_nav_data.Day;

            D("g_nav_data() , Y:%4d, M: %2d , D:%2d", g_nav_data.Year, g_nav_data.Month, g_nav_data.Day);
            D("g_nav_data() , H:%2d, M: %2d , S:%2d", g_nav_data.Hours, g_nav_data.Minutes, g_nav_data.Seconds);

            D("tm() , Y:%4d, M: %2d , D:%2d", fix_tm.tm_year, fix_tm.tm_mon, fix_tm.tm_mday);
            D("tm() , H:%2d, M: %2d , S:%2d", fix_tm.tm_hour, fix_tm.tm_min, fix_tm.tm_sec);

            fix_time = mktime( &fix_tm ) - g_utc_diff;
            g_fix.timestamp = (long long)fix_time * 1000;

            //if (g_fix.flags != 0) {
                //report location
                if ((g_gps_state->callbacks).location_cb) {
                    (g_gps_state->callbacks).location_cb( &g_fix );
                    //g_fix.flags = 0;
                }  else {
                    D("no callback, keeping data until needed !");
                }

#if GPS_DEBUG
                char   temp[256];
                char   *p   = temp;
                char   *end = p + sizeof(temp);
                struct tm   utc;

                p += snprintf( p, end-p, "sending fix" );
                if (g_fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
                    p += snprintf(p, end-p, " lat=%g lon=%g", g_fix.latitude, g_fix.longitude);
                }
                if (g_fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
                    p += snprintf(p, end-p, " altitude=%g", g_fix.altitude);
                }
                if (g_fix.flags & GPS_LOCATION_HAS_SPEED) {
                    p += snprintf(p, end-p, " speed=%g", g_fix.speed);
                }
                if (g_fix.flags & GPS_LOCATION_HAS_BEARING) {
                    p += snprintf(p, end-p, " bearing=%g", g_fix.bearing);
                }
                if (g_fix.flags & GPS_LOCATION_HAS_ACCURACY) {
                    p += snprintf(p,end-p, " accuracy=%g", g_fix.accuracy);
                }
                gmtime_r( (time_t*) &g_fix.timestamp, &utc );
                p += snprintf(p, end-p, " time=%s", asctime( &utc ) );
                D("%s", temp);
#endif


                //report status of satellites in view
                if((g_gps_state->callbacks).sv_status_cb) {
                    memset(&g_svStatus, 0, sizeof(g_svStatus));
                    g_svStatus.num_svs = (int) g_nav_data.SatsInView ;
#if GPS_DEBUG
                    D("Satellites in view count: %d, for used: %d", g_svStatus.num_svs, g_nav_data.SatsUsed);
#endif

                    int i = 0 ;
                    for (i = 0; i < g_svStatus.num_svs; i++) {
                        g_svStatus.sv_list[i].prn = g_nav_data.SatsInViewSVid[i];
                        g_svStatus.sv_list[i].snr = g_nav_data.SatsInViewSNR[i];
                        g_svStatus.sv_list[i].elevation = g_nav_data.SatsInViewElev[i];
                        g_svStatus.sv_list[i].azimuth = g_nav_data.SatsInViewAzim[i];
                        if(g_svStatus.sv_list[i].elevation != -99) {
                            g_svStatus.almanac_mask |= 1<<(g_svStatus.sv_list[i].prn-1);
                        }
                        if(g_nav_data.SatsInViewUsed[i]) {
                            g_svStatus.used_in_fix_mask |= (1<<(g_svStatus.sv_list[i].prn -1));
                        }
#if GPS_DEBUG
                        D("Satellites in view %d: prn:%2d, snr:%2.2f, ele:%2.2f, azi:%2.2f , for fix: %d, mask:%x",
                                i, g_svStatus.sv_list[i].prn,  g_svStatus.sv_list[i].snr,
                                g_svStatus.sv_list[i].elevation,  g_svStatus.sv_list[i].azimuth
                                , g_nav_data.SatsInViewUsed[i], g_svStatus.used_in_fix_mask);
#endif
                    }
                    g_svStatus.ephemeris_mask = g_svStatus.almanac_mask;  //do not know how to give a value
                    (g_gps_state->callbacks).sv_status_cb(&g_svStatus);
                }
            //}
        //}
    }
    else if ( (gn_Patch_Status > 0) && (gn_Patch_Status < 7) )
    {
        LOGGPS("begin to load patch : GN_Upload_GNB_Patch_510 : %d", gn_Patch_Status);
        GN_Upload_GNB_Patch( 20 );
    }
  }//if (reciver_status == TRUE)
}

//static const char k_gps_power_dev[]= "/proc/driver/gps_dev";
static const char k_gps_power_dev[]= "/dev/gps_power";
static int gps_hardware_poweroff()
{
    //gns 7560 poweroff: echo 0 > /dev/gps_power
    //char power_state = '0';
    int power_state = 0;
    int fd = open(k_gps_power_dev, O_WRONLY);

    RUNTO;

        LOGGPS("gps_hardware_poweroff");
    if (fd == -1) {
        LOGGPS("can't open <%s> for <%s>\n", k_gps_power_dev, strerror(errno));
        return -1;
    }
    //write(fd, &power_state, sizeof(power_state));
    ioctl(fd, power_state);
    close(fd);

    return 0;
}
static int gps_hardware_poweron()
{
    //gns 7560 poweron: echo 1 > /dev/gps_power
    //char power_state = '1';
    int power_state = 1;
    int fd = open(k_gps_power_dev, O_WRONLY);

    RUNTO;

        LOGGPS("gps_hardware_poweron");
    if (fd == -1) {
        D("can't open <%s> for <%s>\n", k_gps_power_dev, strerror(errno));
        return -1;
    }
    //write(fd, &power_state, sizeof(power_state));
    ioctl(fd, power_state);
    close(fd);

    return 0;
}

static int internal_req(char command)
{
    char cmd = command;
    int ret = 0;
    int retrycount = 0;

    ENSURE(g_gps_state->init == 1);
    ENSURE(g_gps_state->start == 1);

    D("%s: send %d command: ret=%d: %s", __FUNCTION__, command, ret, strerror(errno));

    do {
        ret=write( g_gps_state->control[0], &cmd, sizeof(cmd) );
        retrycount ++;
    } while ((ret < 0) && (errno == EINTR) && (retrycount < MAX_CMD_TRY_COUNT));

    if (ret != sizeof(cmd))
    {
        D("%s: could not send %d command: ret=%d: %s",
          __FUNCTION__, command, ret, strerror(errno));
        return -1;
    }

    return 0;
}

static void sig_handle(int signo)
{
    if(signo==SIGTERM || signo == SIGKILL || signo == SIGABRT){
        printf("!!!!!!!!!hander SIGTERM signo!!!!!!!!!");
        gps_hardware_poweroff();
        exit(1);
    }
}
static void gps_state_done(GpsState*  s)
{
    // tell the thread to quit, and wait for it
    char   cmd = CMD_QUIT;
    void*  dummy;

    RUNTO;

    g_Task_Exit = 1;
    write( s->control[0], &cmd, 1 );

    //wait the thread to exit
    usleep(60000);

    pthread_join(s->thread, &dummy);
    pthread_join(s->task_thread, &dummy);

    // close the control socket pair
    close( s->control[0] ); s->control[0] = -1;
    close( s->control[1] ); s->control[1] = -1;

    //FIXME: before close gps port , we need to shutdown gps .
    GN_GPS_Shutdown();
    g_status.status = GPS_STATUS_ENGINE_OFF;
    (s->callbacks).status_cb(&g_status);
    sleep(2);

    // close connection to the QEMU GPS daemon
    close( s->fd ); s->fd = -1;
#if WRITE_LOG
    GN_Close_Logs();
#endif
    gps_hardware_poweroff();

    s->init = 0;
}

static int epoll_register(int  epoll_fd, int  fd)
{
    struct epoll_event  ev;
    int                 ret, flags;

    RUNTO;

    /* important: make the fd non-blocking */
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
    } while (ret < 0 && errno == EINTR);
    return ret;
}

static long long update_utc_diff(  )
//static int update_utc_diff(  )
{
    time_t         now = time(NULL);
    struct tm      tm_local;
    struct tm      tm_utc;
    long long      time_local, time_utc;

    gmtime_r( &now, &tm_utc );
    localtime_r( &now, &tm_local );

    time_local = tm_local.tm_sec +
                 60*(tm_local.tm_min +
                 60*(tm_local.tm_hour +
                 24*(tm_local.tm_yday +
                 365*tm_local.tm_year)));

    time_utc = tm_utc.tm_sec +
               60*(tm_utc.tm_min +
               60*(tm_utc.tm_hour +
               24*(tm_utc.tm_yday +
               365*tm_utc.tm_year)));

    D("update_utc_diff() utc, Y:%4d, M: %2d , D:%2d", tm_utc.tm_year, tm_utc.tm_mon, tm_utc.tm_mday);
    D("update_utc_diff() utc, H:%2d, M: %2d , S:%2d", tm_utc.tm_hour, tm_utc.tm_min, tm_utc.tm_sec);


    D("update_utc_diff() loc, Y:%4d, M: %2d , D:%2d", tm_local.tm_year, tm_local.tm_mon, tm_local.tm_mday);
    D("update_utc_diff() loc, H:%2d, M: %2d , S:%2d", tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);


    //D("use unsigned long long type time_local = %llu time_utc = %llu", time_local, time_utc);

    g_utc_diff = time_utc - time_local;
    //D("-------- g_utc_diff = %lld -------------", g_utc_diff);

    return g_utc_diff;
}




static void* gps_task_thread(void*  arg)
{
    U4 loop_duration_ms;                   // Duration to the end of the loop [ms]
    U4 loop_start_ms;                      // OS Tick [ms] at the start of the loop

    int sleep_time_ms = 100;
    update_utc_diff();
    D("gps gps_task_thread running %d", g_Task_Exit);
    // now loop
    while ( g_Task_Exit == 0 ) {
        loop_start_ms = GN_GPS_Get_OS_Time_ms();

    	//D("------------------- GN_GPS_message have change by samty -----------------");
        GN_GPS_message();

        // Sleep for long enough to keep a 50ms repeat period round this loop.
        // This does not need to be exact but makes estimating the amount of
        // Patch data it is safe to send easier.
        loop_duration_ms = GN_GPS_Get_OS_Time_ms() - loop_start_ms;
        if ( loop_duration_ms < sleep_time_ms )
        {
            usleep( (sleep_time_ms - loop_duration_ms) * 1000 );
        }
    }
Exit:
    return NULL;
}


static void* gps_state_thread(void*  arg)
{
    GpsState*   state = (GpsState*) arg;
    int         epoll_fd   = epoll_create(1);
    int         started    = 0;
    //int         gps_fd     = state->fd;
    int         control_fd = state->control[1];

    //update_utc_diff();

    // register control file descriptors for polling
    epoll_register(epoll_fd, control_fd);
    //epoll_register(epoll_fd, gps_fd);

    D("gps thread running");

    // now loop
    for (;;) {
        struct epoll_event   events[1];
        int                  ne, nevents;

        nevents = epoll_wait(epoll_fd, events, 1, -1);
        if (nevents < 0) {
            if (errno != EINTR){
                LOGE("epoll_wait() unexpected error: %s", strerror(errno));
            }
            continue;
        }
        //D("gps thread received %d events", nevents);
        for (ne = 0; ne < nevents; ne++) {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                LOGE("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                goto Exit;
            }
            if ((events[ne].events & EPOLLIN) != 0) {
                int  fd = events[ne].data.fd;
                /*
                if (fd == gps_fd)
                {
                    GN_GPS_message();
                }
                else
                */
                if (fd == control_fd)
                {
                    char  cmd = 255;
                    int   ret;
                    D("gps control fd event");
                    do {
                        ret = read( fd, &cmd, 1 );
                    } while (ret < 0 && errno == EINTR);

                    //RUNTO;
                    if (cmd == CMD_QUIT) {
                        D("gps thread quitting on demand");
                        goto Exit;
                    }
                    else if (cmd == CMD_START) {
                        if (!started) {
                            //D("gps thread starting  location_cb=%p", state->callbacks.location_cb);
                            started = 1;
                            g_status.status = GPS_STATUS_SESSION_BEGIN ;
                            (state->callbacks).status_cb(&g_status);
                        }
                    }
                    else if (cmd == CMD_STOP) {
                        //RUNTO;
                        if (started) {
                            D("gps thread stopping");
                            started = 0;
                            g_status.status = GPS_STATUS_SESSION_END;
                            (state->callbacks).status_cb(&g_status);
                        }
                    }
                    RUNTO;
                }
                else
                {
                    LOGE("epoll_wait() returned unkown fd %d ?", fd);
                }
            }
        }
    }
Exit:
    return NULL;
}


static int gps_state_init(GpsState*  state)
{
    RUNTO;
    char   prop[PROPERTY_VALUE_MAX];
    char   device[256];

    LOGD("--- gps_state_init ---\n");
    gps_hardware_poweron();

/*
    LOGGPS("---gps_state_init ->property_get, ro.kernel.android.gps ------");

    if (property_get("ro.kernel.android.gps",prop,"") == 0) {
        D("no kernel-provided gps device name");
        goto Fail;
    }
    if ( snprintf(device, sizeof(device), "/dev/%s", prop) >= (int)sizeof(device) ) {
        LOGE("gps serial device name too long: '%s'", prop);
        goto Fail;
    }
*/

    state->fd = GN_Port_Setup(GNS_DEVICES,115200,0);

    if (state->fd < 0) {
        LOGE("could not open gps serial device %s: %s", GNS_DEVICES, strerror(errno) );
        goto Fail;
    }

#if FIX_TIME
    writeTimeForFixBegin();
#endif
    GPS_INIT();
    D("gps will read from %s", GNS_DEVICES);
    g_Task_Exit = 0;
    if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 ) {
        LOGE("could not create thread control socket pair: %s", strerror(errno));
        goto FailSocket;
    }

    D("gps state initialized \n");
    g_status.status = GPS_STATUS_ENGINE_ON;

    if ( NULL == (state->callbacks).status_cb )
    {
        state->callbacks = g_save_cbs;
    }

    if ( NULL != (state->callbacks).status_cb )
        (state->callbacks).status_cb(&g_status);
    else
        D("Oh~my god!!\n");

    return 0;

FailSocket:

    close( state->fd ); state->fd = -1;

Fail:

    return -1;
}


static int gns_gps_init(GpsCallbacks* callbacks)
{
    LOGD("gns_gps_init\n");
    ENSURE(callbacks != NULL);

    RUNTO;

    g_gps_state->init = 0;
    g_gps_state->start  = 0;
    g_gps_state->control[0] = -1;
    g_gps_state->control[1] = -1;
    g_gps_state->fd         = -1;
    g_gps_state->freq        = 1000;     //1000 ms

    g_gps_state->callbacks   = *callbacks;

    g_save_cbs = *callbacks;

    g_status.status = GPS_STATUS_NONE;
    (g_gps_state->callbacks).status_cb(&g_status);

    g_gps_state->init = 1;
    return 0;

    //FIXME: check performance and power consumption GN_GPS_POWER_ALLOWED_LOW
    //GN_GPS_Set_Power_Mode(GN_GPS_POWER_ALLOWED_MEDIUM);
    //we do nothing at here. init the gps bb at real start, then we can got more low power consumption.
}

static int gns_gps_start()
{

    RUNTO;
    GpsCallbacks* callbacks = &(g_gps_state->callbacks);

    gps_state_init(g_gps_state);

    D("Begin to start gps ...\n");

    //if ( pthread_create( &state->thread, NULL, gps_state_thread, state )!= 0 ) {
    if ( (g_gps_state->thread = callbacks->create_thread_cb( "gn_gps",
          gps_state_thread, (void *)g_gps_state )) < 0 ) {
        LOGE("could not create gps thread gps_state_thread: %s ", strerror(errno));
        goto FailThread;
    }

    g_Task_Exit = 0;
        //if ( pthread_create( &state->task_thread, NULL, gps_task_thread, NULL ) != 0 ) {
    if ( (g_gps_state->task_thread = callbacks->create_thread_cb( "gn_gps",
             gps_task_thread, (void *)g_gps_state )) < 0 ) {
        LOGE("could not create gps thread gps_task_thread: %s", strerror(errno));
        goto FailThread;
    }

    if(internal_req(CMD_START)) {
        RUNTO;
        goto FailThread;
    }

    if(signal(SIGTERM,sig_handle)==SIG_ERR){
        LOGD("can't catch SIGTERM");
    }

    g_gps_state->start = 1;
    return 0;

    FailThread:

    D("gps start fail process\n");
    gns_gps_stop();

    close( g_gps_state->control[0] ); g_gps_state->control[0] = -1;
    close( g_gps_state->control[1] ); g_gps_state->control[1] = -1;
    return -1;
}

static int gns_gps_stop()
{
    LOGD("gns_gps_stop\n");
    ENSURE(g_gps_state->init == 1);
    ENSURE(g_gps_state->start == 1);

    RUNTO;

    g_Task_Exit = 1;
    if(internal_req(CMD_STOP)) {
        RUNTO;
        return -1;
    }

    gps_state_done(g_gps_state);
    g_gps_state->start = 0;

    return 0;
}

static void
gns_gps_set_fix_frequency(int frequency)
{
    ENSURE(g_gps_state->init == 1);
    ENSURE(g_gps_state->start == 1);

    RUNTO;

    g_gps_state->freq        = 1000 * frequency;
    if(internal_req(CMD_FREQ)) {
        RUNTO;
        return ;
    }

    return ;
/*
    s_GN_GPS_Config   Host_GN_GPS_Config;

    if (GN_GPS_Get_Config( &Host_GN_GPS_Config ) == TRUE )
    {
        Host_GN_GPS_Config.FixInterval      = frequency * 1000;

        if( GN_GPS_Set_Config( &Host_GN_GPS_Config ) == TRUE)
        {
            //succ
            return;
        }
    }
    //error
    return;
    */
}

static void gns_gps_cleanup(void)
{
    ENSURE(g_gps_state->init == 1);
    ENSURE(g_gps_state->start == 0);

   (g_gps_state->callbacks).location_cb = NULL;
   (g_gps_state->callbacks).status_cb= NULL;
   (g_gps_state->callbacks).sv_status_cb= NULL;

    g_gps_state->init   = 0;

    return;
    //GN_GPS_Shutdown();
}

static void
gns_gps_delete_aiding_data(GpsAidingData flags)
{
    ENSURE(g_gps_state->init == 1);
    ENSURE(g_gps_state->start == 1);

    RUNTO;

    if(internal_req(CMD_CLEAR_AID)) {
        RUNTO;
        return;
    }

    return;

    //TODO: use GN_GPS_Clear_NV_Data follow flags
    //GN_GPS_Clear_Pos_NV_Store();
}

static int gns_gps_inject_location (double latitude, double longitude, float accuracy)
{
    /* not yet implemented */
    return 0;
}

static int
gns_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    //uncertainty       ntp transmitTime
    //timeReference     when we got the ntp times.
    //time              ntp time

    //gnx call GN_GPS_Read_UTC to get the rtc times. so this func is unuseful.
    RUNTO;
    return 0;
}

static int gns_gps_set_position_mode(GpsPositionMode mode, int fix_frequency)
{
    //the jni call this func int android_location_GpsLocationProvider_start
    //          //android_location_GpsLocationProvider_start()
    //    int result = sGpsInterface->set_position_mode(GPS_POSITION_MODE_STANDALONE, (singleFix ? 0 : fixFrequency));
    //    if (result) {
    //          return result;
    //    }
    //    return (sGpsInterface->start() == 0);
    //FIXME: in future, we must implement this func, the ASSISTED maybe usefull for performances.
    RUNTO;
    return 0;
}

static const void* gns_gps_get_extension(const char* name)
{
    //android need some one likes ""gps-xtra"
    RUNTO;
    return NULL;
}


static const GpsInterface  gnsGpsInterface = {
    sizeof(GpsInterface),
    gns_gps_init,
    gns_gps_start,
    gns_gps_stop,
    gns_gps_cleanup,
    gns_gps_inject_time,
    gns_gps_inject_location,
    gns_gps_delete_aiding_data,
    gns_gps_set_position_mode,
    gns_gps_get_extension,
};

const GpsInterface* gps_get_hardware_interface()
{
    RUNTO;
    return &gnsGpsInterface;
}

const GpsInterface*
gps_get_interface()
{
    return gps_get_hardware_interface();
}
