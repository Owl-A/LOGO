#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <mutex>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "parser.h"
#include "pprint.h"
#include "interpret.h"
#include "cursor.h"

#define WIDTH 600
#define HEIGHT 600

Cursor* cursor;
Interpret* interpreter;
std::mutex mtx;

void get_input() {
  std::cout << "> ";
  char inp[250];
  std::cin.getline(inp, 250);
  Parser parser(inp);
  try{
    std::vector<AST*> ast = parser.parse();
    for ( auto it = ast.begin(); it != ast.end(); ++it) {
      mtx.lock();
      interpreter->execute(*it);
      mtx.unlock();
    }
    for ( auto it = ast.begin(); it != ast.end(); ++it) {
      delete *it;
    }
  } catch (SyntaxError err) {
    std::cout << "OFFENDING LEXEME : " << err.token->lexeme << std::endl;
    std::cout << "MESSAGE          : " << err.msg << std::endl; 
  }
}

void input_thread() {
  while(true) get_input();
}

int main () {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LOGO interpreter", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD\n";
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, WIDTH, HEIGHT);

  cursor = new Cursor(-0.9, -0.9, 0.9, 0.9, 0.01);
  interpreter = new Interpret(cursor->get_queue());

  std::thread input = std::thread(input_thread);

  while(!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    cursor->render_window();
    cursor->render_lines();
    cursor->render_turtle();
    glfwSwapBuffers(window);
    glfwPollEvents();
    mtx.lock();
    cursor->process();
    mtx.unlock();
  }

  input.join();
  glfwTerminate();
}
