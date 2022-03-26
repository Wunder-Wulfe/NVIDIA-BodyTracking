#pragma once
#include "CVirtualDevice.h"

enum class TRACKER_ROLE;

class CVirtualBodyTracker : public CVirtualDevice
{

    size_t m_index;

    CVirtualBodyTracker(const CVirtualBodyTracker &that) = delete;
    CVirtualBodyTracker &operator=(const CVirtualBodyTracker &that) = delete;

    glm::mat4x4 m_lastTransform;
    glm::mat4x4 m_curTransform;
    uint frame;

    const glm::mat4x4 &InterpolatedTransform() const;

    void SetupProperties() override;
public:
    TRACKER_ROLE role;

    void RunFrame() override;

    void UpdateTransform(const glm::mat4x4 &newTransform);

    explicit CVirtualBodyTracker(size_t p_index, TRACKER_ROLE rle);
    ~CVirtualBodyTracker();
};
