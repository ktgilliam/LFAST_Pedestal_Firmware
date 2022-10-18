#pragma once

#include <NetComms.h>

#define SIM_SCOPE_ENABLED 1
#define SLEW_MULT 512

#if SIM_SCOPE_ENABLED
#define SIDEREAL_RATE_RPS (0.000072921)//(15.041067 / 3600.0 * M_PI / 180.0)
#define AZ_SLEW_RATE (SIDEREAL_RATE_RPS*SLEW_MULT)
#define ALT_SLEW_RATE (SIDEREAL_RATE_RPS*SLEW_MULT)

#define SCOPE_PARK_TIME_COUNT 100
#endif

#define MIN_ALT_ANGLE_RAD 0.0
#define MAX_ALT_ANGLE_RAD (M_PI/2.0)

#define DEFAULT_ALT_PARK (M_PI/4.0)

#define TRACK_ERR_THRESH (2*SIDEREAL_RATE_RPS)//(SIDEREAL_RATE_RPS*4.0)
namespace LFAST
{
class MountControl
{
    private:
        double localSiderealTime = 0.0;

        double currentAzPosn = 0.0;
        double currentAltPosn = 0.0;

        // double targetAzPosn = 0.0;
        // double targetAltPosn = 0.0;

        double AltPosnErr = 0.0;
        double AzPosnErr = 0.0;

        double azParkPosn = 0.0;
        double altParkPosn = DEFAULT_ALT_PARK;

        double targetRaPosn = 0.0;
        double targetDecPosn = 0.0;


        double altPosnCmd_rad = 0.0;
        double azPosnCmd_rad = 0.0;
        
        double azRateCmd_rps = 0.0;
        double altRateCmd_rps = 0.0;

        double azOffset = 0.0;
        double altOffset = 0.0;

        double localLatitude = 0.0;
        double localLongitude = 0.0;

        double deltaTimeSec = 0.0;

    protected:
        enum MountStatus
        {
            MOUNT_IDLE,
            MOUNT_PARKED,
            MOUNT_PARKING,
            MOUNT_SLEWING,
            MOUNT_HOMING,
            MOUNT_TRACKING
        };

        MountStatus mountStatus;

    public:
        MountControl();

        void printMountStatus();
#if SIM_SCOPE_ENABLED
        void initSimMount();
        void updateSimMount();
#endif
        void setLatitude(double lat)
        {
            localLatitude = lat;
        }
        void setLongitude(double lon)
        {
            localLongitude = lon;
        }
        bool mountIsIdle()
        {
            return mountStatus == MOUNT_IDLE;
        }
        bool mountIsParked()
        {
            return mountStatus == MOUNT_PARKED;
        }
        bool mountIsParking()
        {
            return mountStatus == MOUNT_PARKING;
        }
        bool mountIsSlewing()
        {
            return mountStatus == MOUNT_SLEWING;
        }
        bool mountIsHoming()
        {
            return mountStatus == MOUNT_HOMING;
        }
        bool mountIsTracking()
        {
            return mountStatus == MOUNT_TRACKING;
        }


        void findHome();
        void park();
        void unpark();
        void updateClock(double);
        void updateTargetCommands();
        void updateTargetRaDec(double ra, double dec);
        void updatePosnErrors();
        void syncRaDec(double ra, double dec);
        void raDecToAltAz(double ra, double dec, double *alt, double *az);
        // void getTargetAltAz(double *alt, double *az);
        // void setTargetAltAz(double alt, double az);
        void abortSlew();

        void getCurrentRaDec(double *ra, double *dec);
        void altAzToHADec(double alt, double az, double *ha, double *dec);

        static std::string getClockStr(double lst);
        // double getCurrentAlt()
        // {
        //     return currentAltPosn - altOffset;
        // }
        // double getCurrentAz()
        // {
        //     return currentAzPosn - azOffset;
        // }
        double getTrackRate();


        enum AXIS
        {
            ALT_AXIS = 0,
            AZ_AXIS = 1,
            RA_AXIS = 2,
            DEC_AXIS = 3
        };
};

}