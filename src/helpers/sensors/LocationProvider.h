#pragma once

#include "Mesh.h"


class LocationProvider {
protected:
#if defined(MESHSMITH_PHOTON_GPS_TIME)
    bool _time_sync_needed = false;
    uint32_t _time_sync_interval = 0;
#else
    bool _time_sync_needed = true;
#endif

public:
    virtual void syncTime() { _time_sync_needed = true; }
#if defined(MESHSMITH_PHOTON_GPS_TIME)
    virtual void cancelTimeSync() { _time_sync_needed = false; }
    virtual void setTimeSyncInterval(uint32_t interval_seconds) { _time_sync_interval = interval_seconds; }
    virtual uint32_t getTimeSyncInterval() const { return _time_sync_interval; }
    virtual bool hasValidTimestamp() { return isValid() && getTimestamp() > 946684800; } // 2000-01-01 UTC
    virtual bool syncTimeNow(mesh::RTCClock* clock) {
        if (clock == NULL || !hasValidTimestamp()) return false;
        clock->setCurrentTime(getTimestamp());
        _time_sync_needed = false;
        return true;
    }
#endif
    virtual bool waitingTimeSync() { return _time_sync_needed; }
    virtual long getLatitude() = 0;
    virtual long getLongitude() = 0;
    virtual long getAltitude() = 0;
    virtual long satellitesCount() = 0;
    virtual bool isValid() = 0;
    virtual long getTimestamp() = 0;
    virtual void sendSentence(const char * sentence);
    virtual void reset() = 0;
    virtual void begin() = 0;
    virtual void stop() = 0;
    virtual void loop() = 0;
    virtual bool isEnabled() = 0;
};
