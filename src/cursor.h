#ifndef __CURSOR_H
#define __CURSOR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ast.h"
#include "shader.h"

#define EPSILON 1e-8

class Cursor{
  unsigned int VAO[3];
  unsigned int VBO[3];
  unsigned int EBO;
  float loX, loY, hiX, hiY;
  float x, y, angle, step;
  std::queue<AST_Atomic*> input_queue;
  Shader shader;

  std::vector<float> buffer_x, buffer_y, buffer_ang, buffer_s;
public:
  Cursor(float loX, float loY, float hiX, float hiY, float step) :
  loX(loX), hiX(hiX), loY(loY), hiY(hiY), step(step),
  shader("shaders/shader.vs", "shaders/shader.fs") {
    x = (loX + hiX) / 2;
    y = (loY + hiY) / 2;
    angle = 0;

    float window[] = {
      loX, loY,
      hiX, loY,
      hiX, hiY,
      hiX, hiY,
      loX, hiY,
      loX, loY
    };

    int windowBound[] = {
      0, 1,
      1, 2,
      3, 4,
      4, 5
    }; 

    float turtle[] = {
        0.00,  0.05,
       -0.05, -0.05,
        0.05, -0.05
    };

    float line[] = {
      0.0, 0.0,
      0.0, step
    };

    glGenVertexArrays(3, VAO);
    glGenBuffers(3,VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(window), window, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(windowBound), windowBound, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(turtle), turtle, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[2]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
  }

  std::queue<AST_Atomic*>* get_queue() {
    return &input_queue;
  }

  void render_window() {
    shader.use();
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec3("Color", glm::vec3(1.0f, 1.0f, 1.0f));
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    shader.setVec3("Color", glm::vec3(0.0f, 0.0f, 0.0f));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

  void render_lines() {
    shader.use();
    shader.setVec3("Color", glm::vec3(0.0f, 0.0f, 0.0f));
    glBindVertexArray(VAO[2]);
    for ( int i = 0; i < buffer_x.size(); i++ ) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(buffer_x[i], buffer_y[i], 0.0f));
      model = glm::rotate(model, glm::radians(buffer_ang[i]), glm::vec3(0.0f, 0.0f, 1.0f));
      model = glm::scale(model, glm::vec3(buffer_s[i]));
      shader.setMat4("model", model);
      glDrawArrays(GL_LINES, 0, 2);
    } 
    glBindVertexArray(0);
  }

  void render_turtle() {
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    shader.setMat4("model", model);
    shader.setVec3("Color", glm::vec3(0.0f, 0.7f, 0.0f));
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
  }

  void process() {
    while(!input_queue.empty()) {
      AST_Atomic* in = input_queue.front();
      _int_process(in->command->type, in->value);
      input_queue.pop();
      delete in;
    }
  }

private:
  float next_hit(float& rsin, float& rcos, int& axis) {
    float cos =   step * glm::cos(glm::radians(angle)); 
    float sin = - step * glm::sin(glm::radians(angle));

    rsin = sin;
    rcos = cos;

    float hitX = std::max( (loX - x)/sin, (hiX - x)/sin );
    float hitY = std::max( (loY - y)/cos, (hiY - y)/cos );

    if (hitX < hitY) {
      axis = 1; // hit Y axis
      return hitX;
    } else {
      axis = 0; // hit X axis
      return hitY;
    }
  }

  void _forward(float value) {
    if (std::abs(value) < EPSILON) {
      return;
    }
    
    float t, sin, cos;
    int axis;

    t = next_hit(sin, cos, axis);
    buffer_x.push_back(x);
    buffer_y.push_back(y);
    buffer_ang.push_back(angle);
    
    if ( t >= value ) {
      buffer_s.push_back(value);
      x += sin * value;
      y += cos * value;
    } else {
      buffer_s.push_back(t);
      x += sin * t;
      y += cos * t;
      if (axis == 1) { // X hit extreme
	if(std::abs(x - loX) < EPSILON) x = hiX;
	else x = loX;
      }else { // Y hit extreme
	if(std::abs(y - loY) < EPSILON) y = hiY;
	else y = loY;
      }
      _forward(value - t);
    } 
  }

  void _int_process(TokenType type, long long int val) {
    float value = (float) val;
    switch(type) {
      case LEFT:
	angle += value;
	break;
      case RIGHT:
	angle -= value;
	break;
      case FORWARD:
	_forward(value);
	break;
      case BACKWARD:
	angle += 180.0f;
	_forward(value);
	angle -= 180.0f;
	break;
    }
  }
};

#endif
