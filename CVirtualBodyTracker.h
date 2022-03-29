#pragma once
#include "CVirtualDevice.h"

enum class TRACKER_ROLE;

//  A virtual body tracker device
//  Allows different joints in the system to be available as tracking devices
class CVirtualBodyTracker : public CVirtualDevice
{
    //  The index for the tracker
    size_t m_index;

    CVirtualBodyTracker(const CVirtualBodyTracker &that) = delete;
    CVirtualBodyTracker &operator=(const CVirtualBodyTracker &that) = delete;

    //  The last transform that was set (used for interpolation)
    glm::mat4x4 m_lastTransform;
    //  The current transform set (used for interpolation)
    glm::mat4x4 m_curTransform;

    bool m_wasSet;

    //  The frame number recorded by the tracker (used for interpolation)
    float frame;
    double m_lCall;
    double m_diff;
    //  Compute the transform based on the currently set values, and interpolate between them using the frame number
    const glm::mat4x4 InterpolatedTransform() const;

    void SetupProperties() override;

    friend CServerDriver;
public:
    //  The role of this body tracker
    TRACKER_ROLE role;

    void RunFrame() override;

    //  Update the tracker with data from the body tracking service
    void UpdateTransform(const glm::mat4x4 &newTransform);

    explicit CVirtualBodyTracker(size_t p_index, TRACKER_ROLE rle);
    ~CVirtualBodyTracker();
};
