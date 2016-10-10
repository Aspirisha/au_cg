#pragma once

#include <GLFW/glfw3.h>
#include <list>
#include <memory>

class Renderer {
protected:
    Renderer() {}
public:
    static void closeRenderer(Renderer *renderer) {
        renderers().remove_if([renderer](auto &p) { return p.get() == renderer; });
    }

    static void dispatchMouseWheel(GLFWwindow *window, double xoffset, double yoffset) {
        for (auto &r : renderers()) {
            r->onMouseWheel(window, xoffset, yoffset);
        }
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        for (auto &r : renderers()) {
            r->onWindowSizeChanged(window, width, height);
        }
    }

    virtual void onMouseWheel(GLFWwindow *window, double xoffset, double yoffset) = 0;
    virtual void onWindowSizeChanged(GLFWwindow* window, int width, int height) = 0;
    virtual void render() = 0;

protected:
    static inline std::list<std::unique_ptr<Renderer>> &renderers()
    {
        static std::list<std::unique_ptr<Renderer>> renderers;
        return renderers;
    }
};