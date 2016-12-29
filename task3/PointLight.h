//
// Created by andy on 12/29/16.
//

#ifndef TASK3_POINTLIGHT_H
#define TASK3_POINTLIGHT_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

struct PointLight {
    PointLight(glm::vec3 pos) : pos(pos) {
        std::vector<int> v = {1, 2 * rand() % 2, 3};
        std::random_shuffle(v.begin(), v.end());
        color_diffuse = {v[0] / 3.0, v[1] / 3.0, v[2] / 3.0};
        color_specular = {1, 1, 1};
        std::vector<float> rotNormal{1 + rand() % 255, rand() % 255, rand() % 255};

        std::random_shuffle(rotNormal.begin(), rotNormal.end());

        light_M = glm::translate ({}, pos);
        light_M = glm::scale (light_M,
                   glm::vec3 (light_radius, light_radius, light_radius));

        bulbM = glm::translate (glm::mat4{}, pos);
        bulbM = glm::scale (bulbM,
                       glm::vec3 (bulb_radius, bulb_radius, bulb_radius));

        constPos = pos;
    }

    void move() {
        glm::vec3 offset(float(B * sin(freqs.x * t + init_angle_x)), 0, A * float(sin(freqs.y * t + init_angle_y)));

        pos = constPos + offset;
        light_M[3] = glm::vec4(pos, 1);
        bulbM[3] = glm::vec4{pos, 1};
        t += dt;
    }

    glm::mat4 light_M;
    float light_radius = 20;
    float bulb_radius = 0.5;
    glm::mat4 bulbM;
    glm::vec3 color_diffuse;
    glm::vec3 color_specular;
    glm::vec3 pos;
    glm::vec3 constPos;
    glm::vec2 freqs = {rand() % 255 / 255.0, rand() % 255 / 255.0};
    float t = 0;
    float dt = 0.01;
    const float A = rand() % 20;
    const float B = rand() % 20;
    const float init_angle_x = glm::radians(float(rand() % 360));
    const float init_angle_y = glm::radians(float(rand() % 360));
};

#endif //TASK3_POINTLIGHT_H
