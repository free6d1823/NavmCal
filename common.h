#ifndef NAVMES3_COMMON_H_
#define NAVMES3_COMMON_H_
#ifdef ANDROID
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#include <stdio.h>
#define  LOGI(...)  fprintf(stdout, __VA_ARGS__)
#define  LOGE(...)  fprintf(stderr, __VA_ARGS__)

#endif

#define MAX_CAMERAS 4   /*<! numbers of cameras, or number of separated area, in AVM */
#define MAX_FP_AREA 4   /*<! number of feature points region in a camera area */
/*!<numbersof total FP in a camera area*/
#define FP_COUNTS			10


/*<! basic data type re-definition */
typedef unsigned char nfByte, *nfPByte;

typedef struct _nfFloat2D{
    float x;
    float y;
}nfFloat2D;

typedef struct _nfFloat3D{
    float x;
    float y;
    float z;
}nfFloat3D;

typedef struct _nfRectF {
    float l;
    float t;
    float r;
    float b;
}nfRectF;

/*!<Fisheye Correction parametters*/
typedef struct _FecParam{
    nfFloat2D ptCenter;		/*!<symmetry center of image relative to top-left of rcInput, in pixels*/
    float	pitch;			/*!<pitch angle in radiun */
    float	yaw;			/*!<rotate with respect to y-axis */
    float	roll;			/*!<rotate with respect to z-axis */
    float	fov;			/*!<hoizontal FOV factor, 0 to PI*/
    float	k1;             /*!<radical coefficient 1 */
    float	k2;             /*!<radical coefficient 2 */
    float a;				/*!<intrinsic arameters, x- scale factor */
    float b;				/*!<intrinsic arameters, y- scale factor */
    float c; 				/*!<intrinsic arameters, skew  */
}FecParam;
/*!<Homograph Transformation parametters*/
typedef struct _HomoParam{
    int fp_index[4];    /*!<feature points indices of FP array, in the homo region*/
    float h[3][3];     /*!<Homography Matrix coef. of the region */
}HomoParam;

typedef struct _AreaSettings {
    nfRectF  range;                  /*!<the coordinates on final normalized image, 1.0x1.0 of this area  */
    FecParam fec;                   /*!<FEC applied to all area */
    nfFloat2D fpt[FP_COUNTS];         /*!<feature points at final image*/
    nfFloat2D fps[FP_COUNTS];         /*!<feature points at rectified image*/
    nfFloat2D fpf[FP_COUNTS];         /*!<feature points at fisheye image*/

    nfRectF	region[MAX_FP_AREA];      /*!<the normalized coordinates of homo_region on final image*/
    HomoParam homo[MAX_FP_AREA];    /*!<homo apply to selected region */
}AreaSettings;

bool    LoadFecParam(FecParam* pFecParam, int nArea);
bool    LoadAreaSettings(AreaSettings* pParam, int nArea);
bool    LoadHomoParam(HomoParam* pParam, int nArea, int nFp);
bool    SaveFecParam(FecParam* pFecParam, int nArea);
bool    SaveHomoParam(HomoParam* pParam, int nArea, int nFp);
bool    SaveAreaSettings(AreaSettings* pParam, int nArea);


#include "./imglab/ImgProcess.h"
class TexProcess;
class MainWindow;
class nfImage;
extern TexProcess* gpTexProcess;
extern MainWindow* gpMainWin;
extern nfImage* gpInputImage;

/**** ImageView messae ID ****
    client use these ID to communicate with ImageWin derived class
    don't care if the ID is not processed
*/


/* inform FpView to set current camera id, pData = 0~3, camera id */
#define MESSAGE_VIEW_SET_CAMERAID   0x0100

/* inform SingleView,FecView to show feature points, pData = 0 hide, 1 show */
#define MESSAGE_VIEW_SHOW_FEATUREPOINTS 0x1000
/* request SingleView do feature points autodetection, pData don't care */
#define MESSAGE_VIEW_DO_AUTODETECTION   0x1001
/* request FecView to apply new FEC parameters from setrtings, pData don't care */
#define MESSAGE_VIEW_UPDATE_FEC   0x1010
/* inform FecView to show feature points, pData = 0 hide, 1 show */
#define MESSAGE_VIEW_SHOW_GRIDELINES 0x1011


#endif //NAVMES3_COMMON_H_

