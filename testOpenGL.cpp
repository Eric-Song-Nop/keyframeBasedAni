#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Model.hpp"
#include "Shader_m.hpp"
#include "camera.h"

#include <iostream>
#include <stdlib.h>
#include <vector>

#include "cubicSpliner.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1800;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 50.0f));
// Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // glfwSetCursorPosCallback(window, mouse_callback);
  // glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading
  // model).
  stbi_set_flip_vertically_on_load(true);

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // build and compile shaders
  // -------------------------
  Shader ourShader("../vertexShader.glsl", "../fragShader.glsl");

  // load models
  // -----------
  // Model ourModel("../resources/thinker-obj/thinker.obj");
     Model ourModel("../resources/nanosuit/nanosuit.obj");

  // draw in wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Setup Dear ImGui context
  const char *glsl_version = "#version 330";
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
                                          //    io.ConfigFlags |=
                                          //    ImGuiConfigFlags_NavEnableGamepad;
                                          //    // Enable Gamepad Controls

  // Setup Dear ImGui style
  // ImGui::StyleColorsDark();
  ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  std::vector<std::pair<float, glm::vec3>> keyframeVec;

  const float MAX_ANIM_TIME = 9.9f;

  float eulerDegree[3] = {0.0, 0.0, 0.0};
  std::vector<float> T;
  std::vector<float> X;
  std::vector<float> Y;
  std::vector<float> Z;
  float keyframeTime = 0.1;
  float animationStartTime = 0.0;
  std::pair<float, glm::vec3> determinedKeyframe;
  float zeros3[3] = {0.0f, 0.0f, 0.0f};
  std::pair<float, glm::vec3> zeroState;
  zeroState.first = 0;
  zeroState.second[0] = 0.0f;
  zeroState.second[1] = 0.0f;
  zeroState.second[2] = 0.0f;

  cubic::Spliner x_spline;
  cubic::Spliner y_spline;
  cubic::Spliner z_spline;

  keyframeVec.push_back(zeroState);
  bool keyframeWritable = true;
  bool animationPlayable = false;
  bool animationPlaying = false;
  int animationStage = 0;

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't forget to enable shader before setting uniforms
    ourShader.use();
    // view/projection transformations
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    // it's a bit too big for our scene, so scale it down

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                     // and append into it.

      ImGui::Text("Animation controller."); // Display some text (you can use a
                                            // format strings too)
      //   ImGui::Checkbox("Demo Window", &show_demo_window);
      //   // Edit bools storing our window open/close state
      //   ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("keyframeTime", &keyframeTime, 0.1f, MAX_ANIM_TIME);
      // Edit 1 float using a slider from 0.0f to 1.0f
      // ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3
      // floats representing a color

      ImGui::InputFloat3("Euler degree", eulerDegree, 1);

      if (ImGui::Button("Save key frame") && keyframeWritable) {
        // Buttons return true when clicked (most widgets
        // return true when edited/activated)
        T.push_back(keyframeTime);
        X.push_back(eulerDegree[0]);
        Y.push_back(eulerDegree[1]);
        Z.push_back(eulerDegree[2]);
        std::cout << T.back() << std::endl;
        animationPlayable = true;
      }
      if (ImGui::Button("Start animation") && animationPlayable) {
        x_spline.setXY(T.size(), T, X);
        y_spline.setXY(T.size(), T, Y);
        z_spline.setXY(T.size(), T, Z);
        keyframeWritable = false;
        animationStartTime = lastFrame;
        animationPlaying = true;
      }
      //   ImGui::SameLine();
      //   ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }

    if (animationPlaying &&
        (currentFrame - animationStartTime) < MAX_ANIM_TIME) {
      // std::pair<float, glm::vec3> lastState=keyframeVec[animationStage];
      // std::pair<float, glm::vec3> targetState=keyframeVec[animationStage +
      // 1];
      model = glm::rotate(
          model, glm::radians(x_spline.getY(currentFrame - animationStartTime)),
          glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(
          model, glm::radians(y_spline.getY(currentFrame - animationStartTime)),
          glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(
          model, glm::radians(z_spline.getY(currentFrame - animationStartTime)),
          glm::vec3(0.0f, 0.0f, 1.0f));
      // if (currentFrame > animationStartTime + targetState.first){
      //   animationStage++;
      //   std::cout<<animationStage<<std::endl;
      // }
      // if (animationStage >= keyframeVec.size() - 1) {
      //   animationPlaying = false;
      //   animationPlayable = false;
      //   keyframeWritable = true;
      //   keyframeVec.clear();
      //   keyframeVec.push_back(zeroState);
      // }
    }
    if (animationPlaying &&
        (currentFrame - animationStartTime) > MAX_ANIM_TIME) {
      animationPlaying = false;
      animationPlayable = false;
      keyframeWritable = true;
      T.clear();
      X.clear();
      Y.clear();
      Z.clear();
      x_spline.clear();
      y_spline.clear();
      z_spline.clear();
    }

    ourShader.setMat4("model", model);
    ourModel.Draw(ourShader);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset =
      lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(yoffset);
}
