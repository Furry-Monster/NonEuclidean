#include "Engine.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"
#include "Level5.h"
#include "Level6.h"
#include "Physical.h"
#include <cfloat>
#include <cmath>
#include <iostream>

Engine *GH_ENGINE = nullptr;
Player *GH_PLAYER = nullptr;
const Input *GH_INPUT = nullptr;
int GH_REC_LEVEL = 0;
int64_t GH_FRAME = 0;

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {
  Engine *eng = static_cast<Engine *>(glfwGetWindowUserPointer(window));
  if (eng) {
    eng->OnKey(key, scancode, action, mods);
  }
}

static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
  Engine *eng = static_cast<Engine *>(glfwGetWindowUserPointer(window));
  if (eng) {
    eng->OnMouseMove(xpos, ypos);
  }
}

static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                int mods) {
  Engine *eng = static_cast<Engine *>(glfwGetWindowUserPointer(window));
  if (eng) {
    eng->OnMouseButton(button, action, mods);
  }
}

static void windowSizeCallback(GLFWwindow *window, int width, int height) {
  Engine *eng = static_cast<Engine *>(glfwGetWindowUserPointer(window));
  if (eng) {
    eng->OnWindowResize(width, height);
  }
}

Engine::Engine()
    : window(nullptr), iWidth(GH_SCREEN_WIDTH), iHeight(GH_SCREEN_HEIGHT),
      isFullscreen(false), lastMouseX(0.0), lastMouseY(0.0), firstMouse(true) {
  GH_ENGINE = this;
  GH_INPUT = &input;

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return;
  }

  CreateGLWindow();
  InitGLObjects();
  SetupInputs();

  player.reset(new Player);
  GH_PLAYER = player.get();

  vScenes.push_back(std::shared_ptr<Scene>(new Level1));
  vScenes.push_back(std::shared_ptr<Scene>(new Level2(3)));
  vScenes.push_back(std::shared_ptr<Scene>(new Level2(6)));
  vScenes.push_back(std::shared_ptr<Scene>(new Level3));
  vScenes.push_back(std::shared_ptr<Scene>(new Level4));
  vScenes.push_back(std::shared_ptr<Scene>(new Level5));
  vScenes.push_back(std::shared_ptr<Scene>(new Level6));

  LoadScene(0);

  sky.reset(new Sky);
}

Engine::~Engine() {
  DestroyGLObjects();
  if (window) {
    glfwDestroyWindow(window);
  }
  glfwTerminate();
}

int Engine::Run() {
  if (!window) {
    return 1;
  }

  // Setup the timer
  const int64_t ticks_per_step = timer.SecondsToTicks(GH_DT);
  int64_t cur_ticks = timer.GetTicks();
  GH_FRAME = 0;

  // Game loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Confine the cursor
    ConfineCursor();

    if (input.key_press['1']) {
      LoadScene(0);
    } else if (input.key_press['2']) {
      LoadScene(1);
    } else if (input.key_press['3']) {
      LoadScene(2);
    } else if (input.key_press['4']) {
      LoadScene(3);
    } else if (input.key_press['5']) {
      LoadScene(4);
    } else if (input.key_press['6']) {
      LoadScene(5);
    } else if (input.key_press['7']) {
      LoadScene(6);
    }

    // Used fixed time steps for updates
    const int64_t new_ticks = timer.GetTicks();
    for (int i = 0; cur_ticks < new_ticks && i < GH_MAX_STEPS; ++i) {
      Update();
      cur_ticks += ticks_per_step;
      GH_FRAME += 1;
      input.EndFrame();
    }
    cur_ticks = (cur_ticks < new_ticks ? new_ticks : cur_ticks);

    // Setup camera for rendering
    const float n =
        GH_CLAMP(NearestPortalDist() * 0.5f, GH_NEAR_MIN, GH_NEAR_MAX);
    main_cam.worldView = player->WorldToCam();
    main_cam.SetSize(iWidth, iHeight, n, GH_FAR);
    main_cam.UseViewport();

    // Render scene
    GH_REC_LEVEL = GH_MAX_RECURSION;
    Render(main_cam, 0, nullptr);
    glfwSwapBuffers(window);
  }

  return 0;
}

void Engine::OnKey(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
  }

  if (key == GLFW_KEY_ENTER && (mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
    ToggleFullscreen();
    return;
  }

  int asciiKey = 0;
  if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
    asciiKey = 'A' + (key - GLFW_KEY_A);
  } else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
    asciiKey = '0' + (key - GLFW_KEY_0);
  } else if (key == GLFW_KEY_SPACE) {
    asciiKey = ' ';
  } else {
    // left un-handled
    return;
  }

  if (asciiKey > 0 && asciiKey < 256) {
    if (action == GLFW_PRESS) {
      input.key[asciiKey] = true;
      input.key_press[asciiKey] = true;
    } else if (action == GLFW_RELEASE) {
      input.key[asciiKey] = false;
    }
  }
}

void Engine::OnMouseMove(double xpos, double ypos) {
  if (firstMouse) {
    lastMouseX = xpos;
    lastMouseY = ypos;
    firstMouse = false;
    return;
  }

  float xoffset = static_cast<float>(xpos - lastMouseX);
  float yoffset = static_cast<float>(lastMouseY - ypos);

  lastMouseX = xpos;
  lastMouseY = ypos;

  input.mouse_ddx += xoffset;
  input.mouse_ddy += yoffset;
}

void Engine::OnMouseButton(int button, int action, int mods) {
  if (button >= 0 && button < 3) {
    if (action == GLFW_PRESS) {
      input.mouse_button[button] = true;
      input.mouse_button_press[button] = true;
    } else if (action == GLFW_RELEASE) {
      input.mouse_button[button] = false;
    }
  }
}

void Engine::OnWindowResize(int width, int height) {
  iWidth = width;
  iHeight = height;
  // Update viewport immediately
  glViewport(0, 0, width, height);
}

void Engine::LoadScene(int ix) {
  // Clear out old scene
  if (curScene) {
    curScene->Unload();
  }
  vObjects.clear();
  vPortals.clear();
  player->Reset();

  // Create new scene
  curScene = vScenes[ix];
  curScene->Load(vObjects, vPortals, *player);
  vObjects.push_back(player);
}

void Engine::Update() {
  // Update
  for (size_t i = 0; i < vObjects.size(); ++i) {
    assert(vObjects[i].get());
    vObjects[i]->Update();
  }

  // Collisions
  // For each physics object
  for (size_t i = 0; i < vObjects.size(); ++i) {
    Physical *physical = vObjects[i]->AsPhysical();
    if (!physical) {
      continue;
    }
    Matrix4 worldToLocal = physical->WorldToLocal();

    // For each object to collide with
    for (size_t j = 0; j < vObjects.size(); ++j) {
      if (i == j) {
        continue;
      }
      Object &obj = *vObjects[j];
      if (!obj.mesh) {
        continue;
      }

      // For each hit sphere
      for (size_t s = 0; s < physical->hitSpheres.size(); ++s) {
        // Brings point from collider's local coordinates to hits's local
        // coordinates.
        const Sphere &sphere = physical->hitSpheres[s];
        Matrix4 worldToUnit = sphere.LocalToUnit() * worldToLocal;
        Matrix4 localToUnit = worldToUnit * obj.LocalToWorld();
        Matrix4 unitToWorld = worldToUnit.Inverse();

        // For each collider
        for (size_t c = 0; c < obj.mesh->colliders.size(); ++c) {
          Vector3 push;
          const Collider &collider = obj.mesh->colliders[c];
          if (collider.Collide(localToUnit, push)) {
            // If push is too small, just ignore
            push = unitToWorld.MulDirection(push);
            vObjects[j]->OnHit(*physical, push);
            physical->OnCollide(*vObjects[j], push);

            worldToLocal = physical->WorldToLocal();
            worldToUnit = sphere.LocalToUnit() * worldToLocal;
            localToUnit = worldToUnit * obj.LocalToWorld();
            unitToWorld = worldToUnit.Inverse();
          }
        }
      }
    }
  }

  // Portals
  for (size_t i = 0; i < vObjects.size(); ++i) {
    Physical *physical = vObjects[i]->AsPhysical();
    if (physical) {
      for (size_t j = 0; j < vPortals.size(); ++j) {
        if (physical->TryPortal(*vPortals[j])) {
          break;
        }
      }
    }
  }
}

void Engine::Render(const Camera &cam, GLuint curFBO,
                    const Portal *skipPortal) {
  // Check for OpenGL errors before rendering
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    std::cerr << "OpenGL error before Render: " << err << std::endl;
  }

  // Clear buffers
  if (GH_USE_SKY) {
    glClear(GL_DEPTH_BUFFER_BIT);
    sky->Draw(cam);
    err = glGetError();
    if (err != GL_NO_ERROR) {
      std::cerr << "OpenGL error after sky draw: " << err << std::endl;
    }
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Create queries (if applicable)
  GLuint queries[GH_MAX_PORTALS];
  GLuint drawTest[GH_MAX_PORTALS];
  assert(vPortals.size() <= GH_MAX_PORTALS);
  if (occlusionCullingSupported) {
    glGenQueries((GLsizei)vPortals.size(), queries);
  }

  // Draw scene
  for (size_t i = 0; i < vObjects.size(); ++i) {
    vObjects[i]->Draw(cam, curFBO);
  }

  // Draw portals if possible
  if (GH_REC_LEVEL > 0) {
    // Draw portals
    GH_REC_LEVEL -= 1;
    if (occlusionCullingSupported && GH_REC_LEVEL > 0) {
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glDepthMask(GL_FALSE);
      for (size_t i = 0; i < vPortals.size(); ++i) {
        if (vPortals[i].get() != skipPortal) {
          glBeginQuery(GL_SAMPLES_PASSED, queries[i]);
          vPortals[i]->DrawPink(cam);
          glEndQuery(GL_SAMPLES_PASSED);
        }
      }
      for (size_t i = 0; i < vPortals.size(); ++i) {
        if (vPortals[i].get() != skipPortal) {
          glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT, &drawTest[i]);
        }
      };
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glDepthMask(GL_TRUE);
      glDeleteQueries((GLsizei)vPortals.size(), queries);
    }
    for (size_t i = 0; i < vPortals.size(); ++i) {
      if (vPortals[i].get() != skipPortal) {
        if (occlusionCullingSupported && (GH_REC_LEVEL > 0) &&
            (drawTest[i] == 0)) {
          continue;
        } else {
          vPortals[i]->Draw(cam, curFBO);
        }
      }
    }
    GH_REC_LEVEL += 1;
  }

#if 0
    //Debug draw colliders
    for (size_t i = 0; i < vObjects.size(); ++i) {
        vObjects[i]->DebugDraw(cam);
    }
#endif
}

void Engine::CreateGLWindow() {
  // Request OpenGL 3.2 or higher (shaders use version 150)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for Mac OS X
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(iWidth, iHeight, GH_TITLE, nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, this);

  glfwSetKeyCallback(window, keyCallback);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetWindowSizeCallback(window, windowSizeCallback);

  if (GH_HIDE_MOUSE) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  if (GH_START_FULLSCREEN) {
    ToggleFullscreen();
  }
}

void Engine::InitGLObjects() {
  // Initialize extensions - must be called after creating OpenGL context
  glewExperimental = GL_TRUE; // Needed for core profile
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err)
              << std::endl;
    return;
  }

  // Clear any errors from glewExperimental
  GLenum glErr = glGetError();
  if (glErr != GL_NO_ERROR) {
    std::cerr << "OpenGL error after GLEW init: " << glErr << std::endl;
  }

  // Print OpenGL version info
  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cout << "OpenGL Renderer: " << renderer << std::endl;
  std::cout << "OpenGL Version: " << version << std::endl;
  std::cout << "GLSL Version: " << glslVersion << std::endl;

  // Basic global variables
  glClearColor(0.6f, 0.9f, 1.0f, 1.0f);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDepthMask(GL_TRUE);

  // Check GL functionality - use standard OpenGL 3.3+ functions if available
  occlusionCullingSupported = 0;
  GLint queryBits = 0;
  glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &queryBits);
  if (queryBits > 0) {
    occlusionCullingSupported = 1;
  }

  // Check for OpenGL errors
  glErr = glGetError();
  if (glErr != GL_NO_ERROR) {
    std::cerr << "OpenGL error during initialization: " << glErr << std::endl;
  }

  // Attempt to enable vsync
  glfwSwapInterval(1);
}

void Engine::DestroyGLObjects() {
  if (curScene) {
    curScene->Unload();
  }
  vObjects.clear();
  vPortals.clear();
}

void Engine::SetupInputs() {}

void Engine::ConfineCursor() {
  if (GH_HIDE_MOUSE && window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    double centerX = width / 2.0;
    double centerY = height / 2.0;
    glfwSetCursorPos(window, centerX, centerY);
    lastMouseX = centerX;
    lastMouseY = centerY;
  }
}

float Engine::NearestPortalDist() const {
  float dist = FLT_MAX;
  for (size_t i = 0; i < vPortals.size(); ++i) {
    dist = GH_MIN(dist, vPortals[i]->DistTo(player->pos));
  }
  return dist;
}

void Engine::ToggleFullscreen() {
  isFullscreen = !isFullscreen;
  if (isFullscreen) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                         mode->refreshRate);
    iWidth = mode->width;
    iHeight = mode->height;
  } else {
    glfwSetWindowMonitor(window, nullptr, GH_SCREEN_X, GH_SCREEN_Y,
                         GH_SCREEN_WIDTH, GH_SCREEN_HEIGHT, 0);
    iWidth = GH_SCREEN_WIDTH;
    iHeight = GH_SCREEN_HEIGHT;
  }
}
