#pragma once

#include "CVirtualDevice.h"

enum class TRACKER_ROLE;

class CVirtualBodyTracker : public CVirtualDevice
{

    size_t m_index;

    CVirtualBodyTracker(const CVirtualBodyTracker& that) = delete;
    CVirtualBodyTracker& operator=(const CVirtualBodyTracker& that) = delete;

    void SetupProperties() override;
public:
    TRACKER_ROLE role;

    explicit CVirtualBodyTracker(size_t p_index, TRACKER_ROLE rle);
    ~CVirtualBodyTracker();
};

