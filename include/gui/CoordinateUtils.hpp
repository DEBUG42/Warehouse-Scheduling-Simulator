#pragma once
#include <SFML/Graphics.hpp>
#include <cmath> // For M_PI, std::cos, std::sin, std::abs

#ifndef M_PI
// Define M_PI if not already defined (e.g., on Windows with MSVC)
#define M_PI 3.14159265358979323846
#endif

namespace CoordinateUtils
{

    /**
     * @brief Checks if a point is inside a rectangle rotated around its center.
     *
     * @param rectCenter The center of the rectangle in a consistent coordinate system.
     * @param rectSize The width (x) and height (y) of the rectangle.
     * @param rectRotationDegrees The rotation of the rectangle in degrees.
     * @param point The point to test, in the same coordinate system as rectCenter.
     * @return true if the point is inside the rotated rectangle, false otherwise.
     */
    static inline bool isPointInRotatedRect(
        const sf::Vector2f &rectCenter,
        const sf::Vector2f &rectSize,
        float rectRotationDegrees,
        const sf::Vector2f &point)
    {
        // Translate point to be relative to the rectangle's center
        sf::Vector2f p = point - rectCenter;

        // Rotate point in the opposite direction of the rectangle's rotation
        // to align it with the rectangle's local axes.
        float angleRad = -rectRotationDegrees * (static_cast<float>(M_PI) / 180.0f); // Convert degrees to radians
        float cosAngle = std::cos(angleRad);
        float sinAngle = std::sin(angleRad);

        sf::Vector2f rotatedP;
        rotatedP.x = p.x * cosAngle - p.y * sinAngle;
        rotatedP.y = p.x * sinAngle + p.y * cosAngle;

        // Check if the rotated point is within the AABB of the unrotated rectangle
        // (which is now axis-aligned with its center at origin)
        if (std::abs(rotatedP.x) <= rectSize.x / 2.0f &&
            std::abs(rotatedP.y) <= rectSize.y / 2.0f)
        {
            return true;
        }
        return false;
    }

    // Add other utility functions here as needed, for example:
    // static inline sf::Vector2f globalToLocal(const sf::Vector2f& globalPoint, const sf::Transformable& item) {
    //     return item.getInverseTransform().transformPoint(globalPoint);
    // }

    // static inline sf::Vector2f localToGlobal(const sf::Vector2f& localPoint, const sf::Transformable& item) {
    //     return item.getTransform().transformPoint(localPoint);
    // }

} // namespace CoordinateUtils