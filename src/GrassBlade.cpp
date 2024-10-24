#include "GrassBlade.h"

Blade::Blade()
{
	p0 = position;
	p1 = p0 + glm::vec3(0, 0, height);
	p2 = p1 + direction * height * lean;
}

glm::vec3 Blade::calculatePositionAlongQuadraticBezierCurve(float t)
{
	glm::vec3 p0p1 = bezier::lerp(p0, p1, t);
	glm::vec3 p1p2 = bezier::lerp(p1, p2, t);
	return bezier::lerp(p0p1, p1p2, t);
}
