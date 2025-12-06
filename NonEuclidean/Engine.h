#pragma once
#include "Camera.h"
#include "GameHeader.h"
#include "Input.h"
#include "Object.h"
#include "Player.h"
#include "Portal.h"
#include "Scene.h"
#include "Sky.h"
#include "Timer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

class Engine {
public:
  Engine();
  ~Engine();

  int Run();
  void Update();
  void Render(const Camera &cam, GLuint curFBO, const Portal *skipPortal);
  void LoadScene(int ix);

  void OnKey(int key, int scancode, int action, int mods);
  void OnMouseMove(double xpos, double ypos);
  void OnMouseButton(int button, int action, int mods);
  void OnWindowResize(int width, int height);

  const Player &GetPlayer() const { return *player; }
  float NearestPortalDist() const;

private:
  void CreateGLWindow();
  void InitGLObjects();
  void DestroyGLObjects();
  void SetupInputs();
  void ConfineCursor();
  void ToggleFullscreen();

  GLFWwindow *window; // GLFW window
  int iWidth;         // window width
  int iHeight;        // window height
  bool isFullscreen;  // fullscreen state
  double lastMouseX;
  double lastMouseY;
  bool firstMouse;

  Camera main_cam;
  Input input;
  Timer timer;

  std::vector<std::shared_ptr<Object>> vObjects;
  std::vector<std::shared_ptr<Portal>> vPortals;
  std::shared_ptr<Sky> sky;
  std::shared_ptr<Player> player;

  GLint occlusionCullingSupported;

  std::vector<std::shared_ptr<Scene>> vScenes;
  std::shared_ptr<Scene> curScene;
};
