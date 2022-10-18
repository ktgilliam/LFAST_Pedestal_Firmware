#include "Telescope.h"

#include <cmath>

#include <NetComms.h>
#include <mathFuncs.h>
#include <device.h>
#include <debug.h>

LFAST::MountControl::MountControl()
{
#if SIM_SCOPE_ENABLED
    initSimMount();
#endif
}

void LFAST::MountControl::updateClock(double lst)
{
    double dt = lst - localSiderealTime;
    if (dt > 0.0)
    {
        deltaTimeSec = dt * 3600.0;
        localSiderealTime = lst;
    }
}

std::string LFAST::MountControl::getClockStr(double lst)
{
    auto wholeHours = static_cast<unsigned int>(lst);
    auto minutesSeconds = (lst - wholeHours) * 60.0;
    auto wholeMinutes = static_cast<unsigned int>(minutesSeconds);
    auto seconds = (minutesSeconds - wholeMinutes) * 60.0;
    auto wholeSeconds = static_cast<unsigned int>((seconds));
    std::stringstream ss;
    ss << wholeHours << ":" << wholeMinutes << ":" << wholeSeconds;

    return ss.str();
}

void LFAST::MountControl::printMountStatus()
{
    // TEST_SERIAL.printf("\033[32m");
    TEST_SERIAL.printf("\033[%u;%uH", 10, 0);
    TEST_SERIAL.printf("\033[37mLocal Sidereal Time:\033[22G%s\r\n", getClockStr(this->localSiderealTime).c_str());
    TEST_SERIAL.printf("\033[0K\033[37mMount Status:\033[22G");
    switch (this->mountStatus)
    {
    case LFAST::MountControl::MOUNT_IDLE:
        TEST_SERIAL.println("\033[33mIDLE");
        break;
    case LFAST::MountControl::MOUNT_PARKING:
        TEST_SERIAL.println("\033[31mPARKING");
        break;
    case LFAST::MountControl::MOUNT_HOMING:
        TEST_SERIAL.println("\033[31mHOMING");
        break;
    case LFAST::MountControl::MOUNT_SLEWING:
        TEST_SERIAL.println("\033[32mSLEWING");
        break;
    case LFAST::MountControl::MOUNT_PARKED:
        TEST_SERIAL.println("\033[37mPARKED");
        break;
    case LFAST::MountControl::MOUNT_TRACKING:
        TEST_SERIAL.println("\033[37mTRACKING");
        break;
    }
    TEST_SERIAL.println();

    TEST_SERIAL.printf("\033[37mCurrent Altitude:\033[20G%8.4f\033[0K\r\n", rad2deg(this->currentAltPosn));
    TEST_SERIAL.printf("\033[37mTarget Altitude:\033[20G%8.4f\033[0K\r\n", rad2deg(altPosnCmd_rad));
    TEST_SERIAL.printf("\033[37mAltitude Rate:\033[20G%8.4f\033[0K\r\n", rad2deg(altRateCmd_rps));
    TEST_SERIAL.println();

    TEST_SERIAL.printf("\033[37mCurrent Azimuth:\033[20G%8.4f\033[0K\r\n", rad2deg(this->currentAzPosn));
    TEST_SERIAL.printf("\033[37mTarget Azimuth:\033[20G%8.4f\033[0K\r\n", rad2deg(azPosnCmd_rad));
    TEST_SERIAL.printf("\033[37mAzimuth Rate:\033[20G%8.4f\033[0K\r\n", rad2deg(azRateCmd_rps));
    TEST_SERIAL.println();
}

void LFAST::MountControl::findHome()
{
    mountStatus = LFAST::MountControl::MOUNT_HOMING;
#if SIM_SCOPE_ENABLED
    // targetAzPosn = 0.0;
    // targetAltPosn = 0.0;
#else
#warning HOMING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::park()
{
    mountStatus = LFAST::MountControl::MOUNT_PARKING;
    // targetAltPosn = altParkPosn;
    // targetAzPosn = azParkPosn;
#if SIM_SCOPE_ENABLED
#else
#warning PARKING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::unpark()
{
    mountStatus = LFAST::MountControl::MOUNT_IDLE;
#if SIM_SCOPE_ENABLED
#else
#warning UNPARKING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::updateTargetCommands()
{
    // double altRadTmp = saturate(altRad, MIN_ALT_ANGLE_RAD, MAX_ALT_ANGLE_RAD);
    // if (altRadTmp != altRad)
    // {
    //     CURSOR_TO_DEBUG_ROW(3);
    //     TEST_SERIAL.printf("Out of bounds error: %8.4f:%8.4f", rad2deg(altRad), rad2deg(altRadTmp));
    // }
    CURSOR_TO_DEBUG_ROW(0);
    TEST_SERIAL.println("Updating commands...");

    switch (this->mountStatus)
    {
    case LFAST::MountControl::MOUNT_IDLE:
    CURSOR_TO_DEBUG_ROW(3);  TEST_SERIAL.println("idle...");
        break;

    case LFAST::MountControl::MOUNT_PARKING:

    CURSOR_TO_DEBUG_ROW(3);  TEST_SERIAL.println("parking...");
        altPosnCmd_rad = altParkPosn;
        azPosnCmd_rad = azParkPosn;
        updatePosnErrors();

        if (AltPosnErr == 0.0 && AzPosnErr == 0.0)
        {
            mountStatus = MOUNT_PARKED;
            // Intentional fall-through
        }
        else
        {
#if SIM_SCOPE_ENABLED
            altRateCmd_rps = ALT_SLEW_RATE * sign(AltPosnErr);
            azRateCmd_rps = AZ_SLEW_RATE * sign(AzPosnErr);
#endif
            break;
        }
    case LFAST::MountControl::MOUNT_PARKED:
    CURSOR_TO_DEBUG_ROW(3);  TEST_SERIAL.println("parked...");
        altRateCmd_rps = 0.0;
        azRateCmd_rps = 0.0;
        break;
    case LFAST::MountControl::MOUNT_HOMING:

    CURSOR_TO_DEBUG_ROW(3);  TEST_SERIAL.println("homing...");
        altPosnCmd_rad = 0.0;
        azPosnCmd_rad = 0.0;
        updatePosnErrors();
        if (AltPosnErr == 0.0 && AzPosnErr == 0.0)
        {
            mountStatus = MOUNT_IDLE;
            altRateCmd_rps = 0.0;
            azRateCmd_rps = 0.0;
        }
        else
        {
#if SIM_SCOPE_ENABLED
            altRateCmd_rps = ALT_SLEW_RATE * sign(AltPosnErr);
            azRateCmd_rps = AZ_SLEW_RATE * sign(AzPosnErr);
#endif
        }
        break;
    case LFAST::MountControl::MOUNT_SLEWING:
        this->raDecToAltAz(this->targetRaPosn, this->targetDecPosn, &altPosnCmd_rad, &azPosnCmd_rad);
        updatePosnErrors();
        if (std::abs(AltPosnErr) < TRACK_ERR_THRESH && std::abs(AzPosnErr) < TRACK_ERR_THRESH)
        {
            this->mountStatus = MOUNT_TRACKING;
            // Intentional fall-through
        }
        else
        {
            CURSOR_TO_DEBUG_ROW(3);  TEST_SERIAL.printf("slewing...[%8.4f][%8.4f]", std::abs(AltPosnErr), std::abs(AzPosnErr));
#if SIM_SCOPE_ENABLED
            altRateCmd_rps = ALT_SLEW_RATE * sign(AltPosnErr);
            azRateCmd_rps = AZ_SLEW_RATE * sign(AzPosnErr);
#endif
            break;
        }
    case LFAST::MountControl::MOUNT_TRACKING:
    CURSOR_TO_DEBUG_ROW(3);  TEST_SERIAL.println("tracking...");
        this->raDecToAltAz(this->targetRaPosn, this->targetDecPosn, &altPosnCmd_rad, &azPosnCmd_rad);
        updatePosnErrors();
#if SIM_SCOPE_ENABLED
        // NOTE: THIS IS WRONG!!!
        altRateCmd_rps = SIDEREAL_RATE_RPS * sign(AltPosnErr);
        azRateCmd_rps = SIDEREAL_RATE_RPS * sign(AzPosnErr);
#endif
        break;
    }
}

void LFAST::MountControl::updatePosnErrors()
{
    AltPosnErr = altPosnCmd_rad-currentAltPosn;
    AzPosnErr = azPosnCmd_rad-currentAzPosn;
    while (AzPosnErr > M_PI)
    {
        AzPosnErr -= (2 * M_PI);
    }
    while (AzPosnErr < (-1 * M_PI))
    {
        AzPosnErr += (2 * M_PI);
    }
}
void LFAST::MountControl::raDecToAltAz(double ra, double dec, double *alt, double *az)
{
    double ha_rad = ha2rad(localSiderealTime - ra);
    double dec_rad = deg2rad(dec);

    if (ha_rad < 0)
    {
        ha_rad += 2 * M_PI;
    }
    if (ha_rad > M_PI)
    {
        ha_rad = ha_rad - 2 * M_PI;
    }

    double lat_rad = deg2rad(localLatitude);

    double AzTmp = atan2(sin(ha_rad),
                         cos(ha_rad) * sin(lat_rad) - tan(dec_rad) * cos(lat_rad)) -
                   M_PI;
    AzTmp = AzTmp >= 0 ? AzTmp : (AzTmp + 2 * M_PI);

    double AltTmp = asin(sin(lat_rad) * sin(dec_rad) + cos(lat_rad) * cos(dec_rad) * cos(ha_rad));

    *alt = AltTmp;
    *az = AzTmp;
}

void LFAST::MountControl::updateTargetRaDec(double ra, double dec)
{
    targetRaPosn = ra;
    targetDecPosn = dec;
    mountStatus = MOUNT_SLEWING;
}

void LFAST::MountControl::syncRaDec(double ra, double dec)
{
    double newAlt, newAz;
    this->raDecToAltAz(ra, dec, &newAlt, &newAz);
    this->currentAltPosn = newAlt;
    this->currentAzPosn = newAz;
}

double LFAST::MountControl::getTrackRate()
{
    return std::sqrt(azRateCmd_rps * azRateCmd_rps + altRateCmd_rps * altRateCmd_rps) / 3200.0;
}

void LFAST::MountControl::abortSlew()
{
    mountStatus = MOUNT_IDLE;
#if SIM_SCOPE_ENABLED
#else
#warning ABORT (ALT) NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::getCurrentRaDec(double *ra, double *dec)
{
    double haTmp;
    altAzToHADec(currentAltPosn, currentAzPosn, &haTmp, dec);

    if (ra)
        *ra = localSiderealTime - haTmp;
}

void LFAST::MountControl::altAzToHADec(double alt, double az, double *ha, double *dec)
{
    double lat_rad = deg2rad(localLatitude);
    double haTmp = atan2(-sin(az), tan(alt) * cos(lat_rad) - cos(az) * sin(lat_rad));

    if (ha)
        *ha = rad2ha(haTmp);

    double decTmp = asin(sin(lat_rad) * sin(alt) + cos(lat_rad) * cos(alt) * cos(az));

    if (dec)
        *dec = rad2deg(decTmp);
    return;
}

void LFAST::MountControl::initSimMount()
{
    this->mountStatus = MOUNT_PARKED;
    currentAltPosn = altParkPosn;
    currentAzPosn = azParkPosn;
}

void LFAST::MountControl::updateSimMount()
{
    // CURSOR_TO_DEBUG_ROW(0);
    // TEST_SERIAL.println("Updating sim...");
    updateTargetCommands();
    if (std::abs(AzPosnErr) < azRateCmd_rps)
    {
        AzPosnErr = 0.0;
        currentAzPosn = azPosnCmd_rad;
    }
    if (std::abs(AltPosnErr) < altRateCmd_rps)
    {
        AltPosnErr = 0.0;
        currentAltPosn = altPosnCmd_rad;
    }

    auto deltaAlt = altRateCmd_rps * deltaTimeSec;
    auto deltaAz = azRateCmd_rps * deltaTimeSec;

    currentAltPosn += deltaAlt;
    currentAltPosn = saturate(currentAltPosn, MIN_ALT_ANGLE_RAD, MAX_ALT_ANGLE_RAD);

    currentAzPosn += deltaAz;
    if (currentAzPosn > (2 * M_PI))
        currentAzPosn -= (2 * M_PI);
    else if (currentAzPosn < 0)
        currentAzPosn += (2 * M_PI);
}
