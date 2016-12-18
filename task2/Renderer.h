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

    static void dispatchMouseButton(GLFWwindow* window, int button, int action, int mods) {
        for (auto &r : renderers()) {
            r->onMouseButton(window, button, action, mods);
        }
    }

    static void dispatchMousePos(GLFWwindow *window, double x, double y) {
        for (auto &r : renderers()) {
            r->onMousePos(window, x, y);
        }
    }

    static void dispatchKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
        for (auto &r : renderers()) {
            r->onKeyEvent(window, key, scancode, action, mods);
        }
    }

    virtual void onMouseWheel(GLFWwindow *window, double xoffset, double yoffset) = 0;
    virtual void onMouseButton(GLFWwindow *window, int button, int action, int mods) = 0;
    virtual void onWindowSizeChanged(GLFWwindow* window, int width, int height) = 0;
    virtual void onMousePos(GLFWwindow *window, double x, double y) = 0;
    virtual void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
    virtual void render() = 0;

    virtual ~Renderer(){}
protected:
    static inline std::list<std::unique_ptr<Renderer>> &renderers()
    {
        static std::list<std::unique_ptr<Renderer>> renderers;
        return renderers;
    }
};