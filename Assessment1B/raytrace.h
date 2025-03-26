#pragma once


glm::vec3 DoNothing(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir)
{
    return vec3(0);
}

glm::vec3 Shade(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir) {
    vec3 normal = normalize(tri->v1.nor);
    vec3 light_dir = normalize(light_pos - p);
	vec3 colour = vec3(0);

    float ambient = 0.1f;
    vec3 base_color = tri->v1.col;
    colour += ambient * base_color;

    float shadow_t = FLT_MAX;
    vec3 shadow_colour;
    bool in_shadow = false;

    trace(p + 0.01f * light_dir, light_dir, shadow_t, shadow_colour, 0, DoNothing);

    if (shadow_t < length(light_pos - p) - 0.001f) {
        in_shadow = true;
    }

    if (!in_shadow) {
        float diffuse = max(dot(normal, light_dir), 0.0f);
        colour += diffuse * base_color;
    }

    if (tri->reflect && depth < max_recursion_depth) {
        vec3 reflect_dir = reflect(dir, normal);
        vec3 reflect_col = vec3(0);
        float reflect_t = FLT_MAX;

        trace(p + 0.01f * reflect_dir, reflect_dir, reflect_t, reflect_col, depth + 1, Shade);

        float reflectivity = 0.3f;
        colour = mix(colour, reflect_col, reflectivity);
    }

	return colour;
}

float RayPlaneIntersection(glm::vec3 o, glm::vec3 dir, glm::vec3 p, glm::vec3 normal) {
    const float EPSILON = 0.00001f;

    float denom = dot(dir, normal);
    if (fabs(denom) < EPSILON) return FLT_MAX; 

    float t = dot(p - o, normal) / denom;
    
    if (t < 0) { 
        return FLT_MAX;
    }
	return t;
}

bool PointInTriangle(glm::vec3 q, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
    glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);

    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v2;
    glm::vec3 edge3 = v1 - v3;

    if (glm::dot(glm::cross(edge1, q - v1), normal) < 0) return false;
    if (glm::dot(glm::cross(edge2, q - v2), normal) < 0) return false;
    if (glm::dot(glm::cross(edge3, q - v3), normal) < 0) return false;

    return true;
}



float RayTriangleIntersection(glm::vec3 o, glm::vec3 dir, triangle* tri, glm::vec3& point) {
    glm::vec3 v1 = tri->v1.pos;
    glm::vec3 v2 = tri->v2.pos;
    glm::vec3 v3 = tri->v3.pos;

    glm::vec3 normal = normalize(cross(v2 - v1, v3 - v1));

    float t = RayPlaneIntersection(o, dir, v1, normal);
    if (t == FLT_MAX) return FLT_MAX; 

    glm::vec3 q = o +  dir * t;

    if (PointInTriangle(q, v1, v2, v3)) {
        point = q;
        return t;
    }

    return FLT_MAX; 
}

void trace(glm::vec3 o, glm::vec3 dir, float& t, glm::vec3& io_col, int depth, closest_hit p_hit) {
    float closest_t = FLT_MAX;
    triangle* closest_tri = nullptr;
    vec3 hit_point;

    for (auto& tri : tris) {
        vec3 temp_point;
        float t_hit = RayTriangleIntersection(o, dir, &tri, temp_point);
        if (t_hit < closest_t) {
            closest_t = t_hit;
            closest_tri = &tri;
            hit_point = temp_point;
        }
    }

    if (closest_tri) {
        io_col = p_hit(closest_tri, depth, hit_point, dir);
        t = closest_t; 
    }
    else {
        io_col = bkgd;
    }
}



vec3 GetRayDirection(float px, float py, int W, int H, float aspect_ratio, float fov) {
    float x = ((2.0f * (px + 0.5f)) / W) - 1.0f;
	float y = 1.0f - ((2.0f * (py + 0.5f)) / H);
	float scale = tan(fov * 0.5f);

    vec3 forward = vec3(0, 0, -1);
    vec3 right = vec3(1, 0, 0);
    vec3 up = vec3(0, -1, 0);

	vec3 dir = normalize((aspect_ratio * scale * x * right) + (scale * y * up) + forward);

	return dir;
}



void raytrace() {
    float aspect_ratio = (float)PIXEL_W / PIXEL_H;
    float fov = radians(90.0f);

    for (int pixel_y = 0; pixel_y < PIXEL_H; ++pixel_y) {
        float percf = (float)pixel_y / (float)PIXEL_H;
        int perci = percf * 100;
        std::clog << "\rScanlines done: " << perci << "%" << ' ' << std::flush;
        for (int pixel_x = 0; pixel_x < PIXEL_W; ++pixel_x) {
            vec3 ray = GetRayDirection((float)pixel_x, (float)pixel_y, PIXEL_W, PIXEL_H, aspect_ratio, fov);
            vec3 col = vec3(0);
            float t = FLT_MAX;

            trace(eye, ray, t, col, 0, Shade);
            writeCol(col, pixel_x, PIXEL_H - pixel_y - 1);

        }
    }
    std::clog << "\rFinish rendering.           \n";
}

