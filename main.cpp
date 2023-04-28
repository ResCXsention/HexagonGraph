// g++ *.cpp *.c -std=c++17 -lglfw -ldl -lfreetype -I./

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <array>
#include <utility>
#include <cmath>
#include <map>

#if 1
const int screenW = 900;
const int screenH = 900;

void getGLCompileErrors (const unsigned int shader)
{
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
   	std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

const char* loadShaderFile (const char* path)
{
	std::string fileContents;
	std::ifstream file {path};
	char t;
	while (file.get(t)) {
		fileContents.push_back(t);
	}
	file.close();
	char* toReturn = new char[fileContents.size() + 1];
	strcpy(toReturn, fileContents.c_str());
	return toReturn;
}

unsigned int compileAndLinkShader (const char* vertexShaderPath, const char* fragmentShaderPath)
{
	unsigned int shaderProgram { glCreateProgram() };
	unsigned int vertexShader { glCreateShader(GL_VERTEX_SHADER) };
	unsigned int fragmentShader { glCreateShader(GL_FRAGMENT_SHADER) };
	const char* a = loadShaderFile(vertexShaderPath);
	const char* b = loadShaderFile(fragmentShaderPath);
	glShaderSource(vertexShader, 1, &a, NULL);
	glShaderSource(fragmentShader, 1, &b, NULL);
	glCompileShader(vertexShader);
	getGLCompileErrors(vertexShader);
	glCompileShader(fragmentShader);
	getGLCompileErrors(fragmentShader);

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

struct Character
{
	unsigned int texture;
	glm::ivec2 size; // width, height
	glm::ivec2 bearing; // top left corner of glyph
	long int advance;
};

int main ()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(screenW, screenH, "untitled", NULL, NULL);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, screenW, screenH);

	glfwSetFramebufferSizeCallback(window, [] (GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	});

	FT_Library ft;
	FT_Init_FreeType(&ft);
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	// --

	const std::string graphTitle {""};
	const std::string graphSubTitle {""};
	const std::array<std::string, 6> tags { {"", "", "", "", "", ""} };
	const std::array<float, 6> magnitudes { {1.0f, 0.32f, 0.79f, 0.77f, 0.8f, 0.83f} };
	constexpr float hexagonShrink = 0.5f;
	auto xComponent = [&magnitudes] (std::size_t tagIndex, int angle, bool useMagnitude = true) -> float {
		if (useMagnitude == true) return magnitudes.at(tagIndex) * (cos(angle * (M_PI / 180))) * hexagonShrink;
		else return (cos(angle * (M_PI / 180))) * hexagonShrink;
	};

	auto yComponent = [&magnitudes] (std::size_t tagIndex, int angle, bool useMagnitude = true) -> float {
		if (useMagnitude == true) return magnitudes.at(tagIndex) * (sin(angle * (M_PI / 180))) * hexagonShrink;
		else return (sin(angle * (M_PI / 180))) * hexagonShrink;
	};

	// --

	FT_Face labelFace;
	FT_New_Face(ft, "FiraCode-Regular.ttf", 0, &labelFace);
	FT_Set_Pixel_Sizes(labelFace, 0, 22);

	std::map<char, Character> characters;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (unsigned char c = 0; c < 128; c++) {
		FT_Load_Char(labelFace, c, FT_LOAD_RENDER);

		unsigned int characterTex;
		glGenTextures(1, &characterTex);
		glBindTexture(GL_TEXTURE_2D, characterTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, labelFace->glyph->bitmap.width, labelFace->glyph->bitmap.rows, 0, GL_RED,  GL_UNSIGNED_BYTE, labelFace->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character newCharacter {
			characterTex,
			glm::ivec2(labelFace->glyph->bitmap.width, labelFace->glyph->bitmap.rows),
			glm::ivec2(labelFace->glyph->bitmap_left, labelFace->glyph->bitmap_top),
			labelFace->glyph->advance.x
		};

		characters.insert(std::pair<char, Character>(c, newCharacter));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(labelFace);
	FT_Done_FreeType(ft);

	// --

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	float vertices[] {
		0.0f, 0.0f, 0.0f, // centre
		0.0f, magnitudes[0] * hexagonShrink, 0.0f, // top
		xComponent(1, 30), yComponent(1, 30), 0.0f, // top right
		xComponent(2, 30), -yComponent(2, 30), 0.0f, // bottom right
		0.0f, -magnitudes[3] * hexagonShrink, 0.0f, // bottom
		-xComponent(4, 30), -yComponent(4, 30), 0.0f, // bottom left
		-xComponent(5, 30), yComponent(5, 30), 0.0f // top left
	};

	unsigned int indices[] {
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 5,
		0, 5, 6,
		0, 6, 1
	};

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), static_cast<void*>(0));
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned int hexagonShader { compileAndLinkShader("shaders/hexagon.vert", "shaders/hexagon.frag") };

	// --

	unsigned int linesVAO;
	unsigned int linesVBO;
	unsigned int linesEBO;
	unsigned int centreLinesEBO;
	unsigned int centreLinesVAO;
	glGenVertexArrays(1, &linesVAO);
	glGenVertexArrays(1, &centreLinesVAO);
	glGenBuffers(1, &linesVBO);
	glGenBuffers(1, &linesEBO);
	glGenBuffers(1, &centreLinesEBO);

	float linesVertices[12] {
		0.0f, 1.0f * hexagonShrink, // top
		xComponent(1, 30, false), yComponent(1, 30, false), // top right
		xComponent(2, 30, false), -yComponent(2, 30, false), // bottom right
		0.0f, -1.0f * hexagonShrink, // bottom
		-xComponent(4, 30, false), -yComponent(4, 30, false), // bottom left
		-xComponent(5, 30, false), yComponent(5, 30, false) // top left
	};

	// perimeter lines
	unsigned int linesIndices[] {
		0, 1,
		1, 2,
		2, 3,
		3, 4,
		4, 5,
		5, 0,
	};

	// centre lines
	unsigned int centreLinesIndices[] {
		0, 3,
		1, 4,
		2, 5
	};

	glBindVertexArray(linesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, linesEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(linesIndices), linesIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(centreLinesVAO);

	glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, centreLinesEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(centreLinesIndices), centreLinesIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	unsigned int linesShader { compileAndLinkShader("shaders/lines.vert", "shaders/lines.frag") };

	const glm::vec3 perimeterLineColour {glm::vec3(0.05f, 0.05f, 0.05f)};
	const glm::vec3 centreLineColour {glm::vec3(0.25f, 0.25f, 0.25f)};

	// --

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	unsigned int fontsVAO;
	unsigned int fontsVBO;
	glGenVertexArrays(1, &fontsVAO);
	glGenBuffers(1, &fontsVBO);
	glBindVertexArray(fontsVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fontsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned int textShader { compileAndLinkShader("shaders/fonts.vert", "shaders/fonts.frag") };

	glm::mat4 projection {glm::ortho(0.0f, static_cast<float>(screenW), static_cast<float>(screenH), 0.0f)};
	glUseProgram(textShader);
	glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, false, glm::value_ptr(projection));
	glUseProgram(0);

	auto rotationMatrix { [] (glm::vec2 position, int rotation) -> const glm::mat4
	{
		glm::mat4 positionMatrix {glm::mat4(1.0f)};
		positionMatrix *= glm::translate(positionMatrix, {(screenW / 2) - -position.x, (screenH / 2) - -position.y, 0.0});
		positionMatrix *= glm::rotate(positionMatrix, static_cast<float>(rotation * (M_PI / 180)), {0.0, 0.0, -1.0});
		positionMatrix *= glm::translate(positionMatrix, {(screenW / 2) - position.x, (screenH / 2) - position.y, 0.0});
		return positionMatrix;
	} };

	// renders text centred
	auto renderText { [&] (std::string text, glm::vec2 position, int rotation) -> void
	{
		glUseProgram(textShader);
		glBindVertexArray(fontsVAO);
		glActiveTexture(GL_TEXTURE0);

		glm::vec2 letterOffset {glm::vec2(0.0f, 0.0f)};
		float xOffset = 0.0f;
		std::string::const_iterator c;

		auto advanceLengthInPixels {[] (long int value) {
			return (value >> 6);
		}};
		int totalWidthOfTextString {0};
		for (c = text.begin(); c != text.end(); c++) {
			totalWidthOfTextString += advanceLengthInPixels(characters[*c].advance);
		}
		int halfWidthOfTextString {totalWidthOfTextString / 2};

		for (c = text.begin(); c != text.end(); c++) {
			Character character = characters[*c];

			float x = character.bearing.x;
			float y = character.size.y - character.bearing.y;

			float w = character.size.x;
       	float h = character.size.y;

			float vertices[24] {
				0 + xOffset, y, 0.0f, 1.0f,
				w + xOffset, y, 1.0f, 1.0f,
				0 + xOffset, y - h, 0.0f, 0.0f,

				w + xOffset, y, 1.0f, 1.0f,
				w + xOffset, y - h, 1.0f, 0.0f,
				0 + xOffset, y - h, 0.0f, 0.0f
			};

			glm::mat4 transformation {glm::mat4(1.0f)};
			transformation *= glm::translate(glm::mat4(1.0f), glm::vec3(position.x - halfWidthOfTextString, position.y, 0.0f));
			transformation *= glm::rotate(glm::mat4(1.0f), static_cast<float>(rotation * (M_PI / 180)), glm::vec3(0.0f, 0.0f, -1.0f));
			//transformation *= glm::translate(glm::mat4(1.0f), glm::vec3(letterOffset.x, letterOffset.y, 0.0f));
			glUniformMatrix4fv(glGetUniformLocation(textShader, "transform"), 1, false, glm::value_ptr(transformation));
			
			glBindTexture(GL_TEXTURE_2D, character.texture);
			glBindBuffer(GL_ARRAY_BUFFER, fontsVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			xOffset += advanceLengthInPixels(characters[*c].advance);
			letterOffset = glm::vec2(xOffset * cos(rotation * (M_PI / 180)), xOffset * sin(rotation * (M_PI / 180)));
			//position.x += advanceLeng// --
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	} };

	auto screenCentre {[] () -> const glm::vec2 {
		return glm::vec2(screenW / 2.0f, screenH / 2.0f);
	}};

	auto screenCentreOffset {[] (const float xOffset, const float yOffset) -> const glm::vec2 {
		return glm::vec2((screenW / 2.0f) + xOffset, (screenH / 2.0f) + yOffset);
	}};

	constexpr float textDistFromCentre = 300.0f;
	constexpr int textRotation = 0;

	// --

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// --

		glBindVertexArray(VAO);
		glUseProgram(hexagonShader);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

		glUseProgram(linesShader);
		glLineWidth(1.0f);
		glBindVertexArray(centreLinesVAO);
		glUniform3fv(glGetUniformLocation(linesShader, "lineColour"), 1, glm::value_ptr(centreLineColour));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, centreLinesEBO);
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);

		glUseProgram(linesShader);
		glLineWidth(3.0f);
		glUniform3fv(glGetUniformLocation(linesShader, "lineColour"), 1, glm::value_ptr(perimeterLineColour));
		glBindVertexArray(linesVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, linesEBO);
		glDrawElements(GL_LINES, 18, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		renderText(tags[0], screenCentreOffset(0.0f, -textDistFromCentre), 0);
		renderText(tags[1], screenCentreOffset(textDistFromCentre * cos(30 * (M_PI / 180)), textDistFromCentre * sin(-30 * (M_PI / 180))), -textRotation);
		renderText(tags[2], screenCentreOffset(textDistFromCentre * cos(-30 * (M_PI / 180)), textDistFromCentre * -sin(-30 * (M_PI / 180))), textRotation);
		renderText(tags[3], screenCentreOffset(0.0f, textDistFromCentre), 0);
		renderText(tags[4], screenCentreOffset(textDistFromCentre * -cos(30 * (M_PI / 180)), textDistFromCentre * sin(30 * (M_PI / 180))), -textRotation);
		renderText(tags[5], screenCentreOffset(textDistFromCentre * -cos(30 * (M_PI / 180)), textDistFromCentre * sin(-30 * (M_PI / 180))), textRotation);

		renderText(graphTitle, glm::vec2(200.0f, 50.0f), 0);
		renderText(graphSubTitle, glm::vec2(200.0f, 85.0f), 0);

		// --

		glfwSwapBuffers(window);
	}

	glDeleteProgram(hexagonShader);
	glDeleteProgram(textShader);
	unsigned int VAOsToDelete[] {VAO, fontsVAO, linesVAO, centreLinesVAO};
	glDeleteVertexArrays(2, VAOsToDelete);
	unsigned int buffersToDelete[] {EBO, VBO, fontsVBO, linesVBO, linesEBO, centreLinesEBO};
	glDeleteBuffers(3, buffersToDelete);
	glfwTerminate();

	return 0;
}
#endif