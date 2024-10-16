#include "GrassBlade.h"

Blade::Blade()
{
	p0 = position;
	p1 = p0 + glm::vec3(0, 0, height);
	p2 = p1 + direction * height * lean;
}
