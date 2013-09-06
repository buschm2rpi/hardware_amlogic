/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#ifndef V4L_CAMERA_ADAPTER_H
#define V4L_CAMERA_ADAPTER_H

#include "CameraHal.h"
#include "BaseCameraAdapter.h"
#include "DebugUtils.h"
#include "Encoder_libjpeg.h"

namespace android {

#ifdef AMLOGIC_USB_CAMERA_SUPPORT
#define DEFAULT_PREVIEW_PIXEL_FORMAT        V4L2_PIX_FMT_NV21
//#define DEFAULT_PREVIEW_PIXEL_FORMAT        V4L2_PIX_FMT_YUYV
#define DEFAULT_IMAGE_CAPTURE_PIXEL_FORMAT  V4L2_PIX_FMT_RGB24
#else
#define DEFAULT_PREVIEW_PIXEL_FORMAT        V4L2_PIX_FMT_NV21
#define DEFAULT_IMAGE_CAPTURE_PIXEL_FORMAT  V4L2_PIX_FMT_RGB24
#endif
#define NB_BUFFER 6

struct VideoInfo {
    struct v4l2_capability cap;
    struct v4l2_format format;
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers rb;
    void *mem[NB_BUFFER];
    bool isStreaming;
    int width;
    int height;
    int formatIn;
    int framesizeIn;
};

typedef enum camera_light_mode_e {
    ADVANCED_AWB = 0,
    SIMPLE_AWB,
    MANUAL_DAY,
    MANUAL_A,
    MANUAL_CWF,
    MANUAL_CLOUDY,
}camera_light_mode_t;

typedef enum camera_saturation_e {
    SATURATION_N4_STEP = 0,
    SATURATION_N3_STEP,
    SATURATION_N2_STEP,
    SATURATION_N1_STEP,
    SATURATION_0_STEP,
    SATURATION_P1_STEP,
    SATURATION_P2_STEP,
    SATURATION_P3_STEP,
    SATURATION_P4_STEP,
}camera_saturation_t;


typedef enum camera_brightness_e {
    BRIGHTNESS_N4_STEP = 0,
    BRIGHTNESS_N3_STEP,
    BRIGHTNESS_N2_STEP,
    BRIGHTNESS_N1_STEP,
    BRIGHTNESS_0_STEP,
    BRIGHTNESS_P1_STEP,
    BRIGHTNESS_P2_STEP,
    BRIGHTNESS_P3_STEP,
    BRIGHTNESS_P4_STEP,
}camera_brightness_t;

typedef enum camera_contrast_e {
    CONTRAST_N4_STEP = 0,
    CONTRAST_N3_STEP,
    CONTRAST_N2_STEP,
    CONTRAST_N1_STEP,
    CONTRAST_0_STEP,
    CONTRAST_P1_STEP,
    CONTRAST_P2_STEP,
    CONTRAST_P3_STEP,
    CONTRAST_P4_STEP,
}camera_contrast_t;

typedef enum camera_hue_e {
    HUE_N180_DEGREE = 0,
    HUE_N150_DEGREE,
    HUE_N120_DEGREE,
    HUE_N90_DEGREE,
    HUE_N60_DEGREE,
    HUE_N30_DEGREE,
    HUE_0_DEGREE,
    HUE_P30_DEGREE,
    HUE_P60_DEGREE,
    HUE_P90_DEGREE,
    HUE_P120_DEGREE,
    HUE_P150_DEGREE,
}camera_hue_t;

typedef enum camera_special_effect_e {
    SPECIAL_EFFECT_NORMAL = 0,
    SPECIAL_EFFECT_BW,
    SPECIAL_EFFECT_BLUISH,
    SPECIAL_EFFECT_SEPIA,
    SPECIAL_EFFECT_REDDISH,
    SPECIAL_EFFECT_GREENISH,
    SPECIAL_EFFECT_NEGATIVE,
}camera_special_effect_t;

typedef enum camera_exposure_e {
    EXPOSURE_N4_STEP = 0,
    EXPOSURE_N3_STEP,
    EXPOSURE_N2_STEP,
    EXPOSURE_N1_STEP,
    EXPOSURE_0_STEP,
    EXPOSURE_P1_STEP,
    EXPOSURE_P2_STEP,
    EXPOSURE_P3_STEP,
    EXPOSURE_P4_STEP,
}camera_exposure_t;


typedef enum camera_sharpness_e {
    SHARPNESS_1_STEP = 0,
    SHARPNESS_2_STEP,
    SHARPNESS_3_STEP,
    SHARPNESS_4_STEP,
    SHARPNESS_5_STEP,
    SHARPNESS_6_STEP,
    SHARPNESS_7_STEP,
    SHARPNESS_8_STEP,
    SHARPNESS_AUTO_STEP,
}camera_sharpness_t;

typedef enum camera_mirror_flip_e {
    MF_NORMAL = 0,
    MF_MIRROR,
    MF_FLIP,
    MF_MIRROR_FLIP,
}camera_mirror_flip_t;


typedef enum camera_wb_flip_e {
    CAM_WB_AUTO = 0,
    CAM_WB_CLOUD,
    CAM_WB_DAYLIGHT,
    CAM_WB_INCANDESCENCE,
    CAM_WB_TUNGSTEN,
    CAM_WB_FLUORESCENT,
    CAM_WB_MANUAL,
}camera_wb_flip_t;
typedef enum camera_night_mode_flip_e {
    CAM_NM_AUTO = 0,
	CAM_NM_ENABLE,
}camera_night_mode_flip_t;

typedef enum camera_effect_flip_e {
    CAM_EFFECT_ENC_NORMAL = 0,
	CAM_EFFECT_ENC_GRAYSCALE,
	CAM_EFFECT_ENC_SEPIA,
	CAM_EFFECT_ENC_SEPIAGREEN,
	CAM_EFFECT_ENC_SEPIABLUE,
	CAM_EFFECT_ENC_COLORINV,
}camera_effect_flip_t;

typedef enum camera_flashlight_status_e{
	FLASHLIGHT_AUTO = 0,
	FLASHLIGHT_ON,
	FLASHLIGHT_OFF,
	FLASHLIGHT_TORCH,
}camera_flashlight_status_t;


/**
  * Class which completely abstracts the camera hardware interaction from camera hal
  * TODO: Need to list down here, all the message types that will be supported by this class
                Need to implement BufferProvider interface to use AllocateBuffer of OMX if needed
  */
class V4LCameraAdapter : public BaseCameraAdapter
{
public:

    /*--------------------Constant declarations----------------------------------------*/
    static const int32_t MAX_NO_BUFFERS = 20;

    ///@remarks OMX Camera has six ports - buffer input, time input, preview, image, video, and meta data
    static const int MAX_NO_PORTS = 6;

    ///Five second timeout
    static const int CAMERA_ADAPTER_TIMEOUT = 5000*1000;

public:

    V4LCameraAdapter(size_t sensor_index);
    ~V4LCameraAdapter();


    ///Initialzes the camera adapter creates any resources required
    virtual status_t initialize(CameraProperties::Properties*);
    //virtual status_t initialize(CameraProperties::Properties*, int sensor_index=0);

    //APIs to configure Camera adapter and get the current parameter set
    virtual status_t setParameters(const CameraParameters& params);
    virtual void getParameters(CameraParameters& params);

    // API
    virtual status_t UseBuffersPreview(void* bufArr, int num);
    virtual status_t UseBuffersCapture(void* bufArr, int num);

    //API to flush the buffers for preview
    status_t flushBuffers();

protected:

//----------Parent class method implementation------------------------------------
    virtual status_t takePicture();
    virtual status_t startPreview();
    virtual status_t stopPreview();
    virtual status_t useBuffers(CameraMode mode, void* bufArr, int num, size_t length, unsigned int queueable);
    virtual status_t fillThisBuffer(void* frameBuf, CameraFrame::FrameType frameType);
    virtual status_t getFrameSize(size_t &width, size_t &height);
    virtual status_t getPictureBufferSize(size_t &length, size_t bufferCount);
    virtual status_t getFrameDataSize(size_t &dataFrameSize, size_t bufferCount);
    virtual void onOrientationEvent(uint32_t orientation, uint32_t tilt);
//-----------------------------------------------------------------------------


private:

    class PreviewThread : public Thread {
            V4LCameraAdapter* mAdapter;
        public:
            PreviewThread(V4LCameraAdapter* hw) :
                    Thread(false), mAdapter(hw) { }
            virtual void onFirstRef() {
                run("CameraPreviewThread", PRIORITY_URGENT_DISPLAY);
            }
            virtual bool threadLoop() {
                mAdapter->previewThread();
                // loop until we need to quit
                return true;
            }
        };

    status_t setBuffersFormat(int width, int height, int pixelformat);
    status_t getBuffersFormat(int &width, int &height, int &pixelformat);

    //Used for calculation of the average frame rate during preview
    status_t recalculateFPS();

    char * GetFrame(int &index);

    int previewThread();

    static int beginPictureThread(void *cookie);
    int pictureThread();

	int GenExif(ExifElementsTable* exiftable);

public:

private:
    int mPreviewBufferCount;
    KeyedVector<int, int> mPreviewBufs;
	KeyedVector<int, int> mPreviewIdxs;
    mutable Mutex mPreviewBufsLock;

    //TODO use members from BaseCameraAdapter
    camera_memory_t *mCaptureBuf;

    CameraParameters mParams;

    bool mPreviewing;
    bool mCapturing;
    Mutex mLock;

    int mFrameCount;
    int mLastFrameCount;
    unsigned int mIter;
    nsecs_t mLastFPSTime;

    //variables holding the estimated framerate
    float mFPS, mLastFPS;

    int mSensorIndex;
    bool mbFrontCamera;

    // protected by mLock
    sp<PreviewThread>   mPreviewThread;

    struct VideoInfo *mVideoInfo;
    int mCameraHandle;


    int nQueued;
    int nDequeued;

    int mZoomlevel;

#ifdef AMLOGIC_USB_CAMERA_SUPPORT
    int mUsbCameraStatus;

    enum UsbCameraStatus
    {
        USBCAMERA_NO_INIT,
        USBCAMERA_INITED,
        USBCAMERA_ACTIVED
    };
#endif
    //int maxQueueable;//the max queued buffers in v4l

};
}; //// namespace
#endif //V4L_CAMERA_ADAPTER_H

