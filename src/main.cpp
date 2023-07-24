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
#include "textbox.h"

#define WIDTH 600
#define HEIGHT 640

Cursor* cursor;
Textbox* textbox;
Interpret* interpreter;
std::mutex mtx;

void get_input(const char* inp) {
  Parser parser(inp);
  try{
    std::vector<AST*> ast = parser.parse();
    for ( auto it = ast.begin(); it != ast.end(); ++it) {
      //mtx.lock();
      interpreter->execute(*it);
      //mtx.unlock();
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
  while(true) {
    std::cout << "> ";
    char inp[250];
    std::cin.getline(inp, 250);
    get_input(inp);
  }
}

void process_input(GLFWwindow* window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS){
    textbox->remove_char();
  }
  if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
    std::string inp = textbox->flush();
    std::cout << inp << std::endl;
    get_input(inp.c_str());
  }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
  char ch = (char)(codepoint % 0x100);
  textbox->add_char(ch);
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
  glfwSetCharCallback(window, character_callback);
  glfwSetKeyCallback(window, key_callback);

  cursor = new Cursor(-0.9, -0.7, 0.9, 0.9, 0.01);
  textbox = new Textbox(-0.9, -0.9, 0.9, -0.8, 0.0015); 
  interpreter = new Interpret(cursor->get_queue());

  //std::thread input = std::thread(input_thread);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while(!glfwWindowShouldClose(window)) {
    process_input(window);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    cursor->render_window();
    cursor->render_lines();
    cursor->render_turtle();
    textbox->render_window();
    textbox->render_text();
    glfwSwapBuffers(window);
    glfwPollEvents();
    //mtx.lock();
    cursor->process();
    //mtx.unlock();
  }

  glfwTerminate();
  //input.join();
}
