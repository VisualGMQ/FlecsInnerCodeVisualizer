#pragma once
#include <string>

struct GLFWwindow;

class Context {
public:
    virtual ~Context() = default;
    
    void OnInit();
    void OnUpdate();
    void OnQuit();
    bool ShouldExit() const;

protected:
    virtual void onUpdate() = 0;
    virtual void onInit() = 0;
    virtual void onQuit() = 0;
    
private:
    GLFWwindow *m_window{};

    void initGLFW();
    void initImGui();
    void shutdownGLFW();
    void shutdownImGui();

    float m_main_scale{1};
    std::string m_glsl_version;
};