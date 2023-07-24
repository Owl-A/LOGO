#ifndef __TEXTBOX_H
#define __TEXTBOX_H
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

class TextPack{
  struct Character{
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    long int advance;
  };

  std::map<char, Character> Characters;
  Shader shader;
  unsigned int VAO, VBO;

public:

  void render_text(char* text, int len, float max_adv, float x, float y, float scale, glm::vec3 color) {
    shader.use();
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    int idx = len - 1;
    float acc = 0.0f;

    while (acc < max_adv && idx >= 0) {
      acc += (Characters[text[idx]].advance >> 6) * scale;
      idx--;
    }
    idx++;

    for (;idx < len; idx++) {
      Character ch = Characters[text[idx]];

      float xpos = x + ch.Bearing.x * scale;
      float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

      float w = ch.Size.x * scale;
      float h = ch.Size.y * scale;

      float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f } 
      };

      glBindTexture(GL_TEXTURE_2D, ch.TextureID);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      //glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  TextPack(const char* fontPath)
  : shader("shaders/text.vs", "shaders/text.fs"){
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
      exit(-1);
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      exit(-1);
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYPE: Failed to load Glyph " << (int)c  << std::endl;
	continue;
      }
      unsigned int texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(
	GL_TEXTURE_2D,
	0,
	GL_RED,
	face->glyph->bitmap.width,
	face->glyph->bitmap.rows,
	0,
	GL_RED,
	GL_UNSIGNED_BYTE,
	face->glyph->bitmap.buffer
      );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      Character character = {
	texture,
	glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
	face->glyph->advance.x
      };

      Characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
};


class Textbox {
  unsigned int VAO, VBO, EBO;
  float loX, loY, hiX, hiY, scale;
  char buffer[250];
  int buflen;
  Shader shader;
  TextPack textpack;
public:
  Textbox(float loX, float loY, float hiX, float hiY, float scale) :
  loX(loX), hiX(hiX), loY(loY), hiY(hiY), scale(scale),
  shader("shaders/shader.vs", "shaders/shader.fs"),
  textpack("../fonts/NotoSans.ttf") {

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
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(window), window, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(windowBound), windowBound, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    memset(buffer, 0, 250);
    buflen = 0;
  }

  void add_char(char c) {
    if (buflen >= 250) return;
    buffer[buflen++] = c;
  }

  void remove_char() {
    if (buflen <= 0) return;
    buffer[--buflen] = '\0';
  }

  std::string flush() {
    std::string ret = std::string(buffer);
    memset(buffer, 0, 250);
    buflen = 0;
    return ret;
  }

  void render_window() {
    shader.use();
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec3("Color", glm::vec3(1.0f, 1.0f, 1.0f));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    shader.setVec3("Color", glm::vec3(0.0f, 0.0f, 0.0f));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

  void render_text() {
    textpack.render_text(buffer, buflen, hiX -loX -0.08, loX + 0.015, (0.7 * loY + 0.3 * hiY) , scale, glm::vec3(0.1f, 0.1f, 0.1f));
  }
};

#endif
