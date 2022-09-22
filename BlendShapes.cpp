/*
CPE/CSC 474 Lab base code Eckhardt/Dahl
based on CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "line.h"
#include "sound.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
shared_ptr<Shape> face;
shared_ptr<Shape> o;
shared_ptr<Shape> a;
shared_ptr<Shape> ee;
shared_ptr<Shape> f;
shared_ptr<Shape> l;
shared_ptr<Shape> m;
shared_ptr<Shape> i_face;
shared_ptr<Shape> t_face;
music_ music;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
		//rot = vec3(0, -1.0647, 0);
		//pos = vec3(-2.396, 0, .495);
		rot = vec3(0, 1.085, 0);
		pos = vec3(1.4469, 0.000, 1.587);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10 * ftime;
		}
		else if (s == 1)
		{
			speed = -10 * ftime;
		}
		float yangle = 0;
		if (a == 1)
			yangle = -3 * ftime;
		else if (d == 1)
			yangle = 3 * ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;
	}
};

camera mycam;
float lastX, lastY;
void mouse_curs_callback(GLFWwindow* window, double xpos, double ypos)
{
	lastX = xpos;
	lastY = ypos;
}

class Application : public EventCallbacks
{

public:

	WindowManager* windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, psky, faceprog, bloomP, fBloomP;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox;

	//texture data
	GLuint Texture;
	GLuint Texture2;

	GLuint bFBO, quadVAO, quadVBO, quadVBO2, rboDepth, rboStencil;
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];
	unsigned int colorBuffers[2];

	//line
	Line linerender;
	Line smoothrender;
	vector<vec3> line;
	int musicID = 0;


	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}


		if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			if (smoothrender.is_active())
				smoothrender.reset();
			else
			{
				vector<vec3> cardinal;
				cardinal_curve(cardinal, line, 5, 1.0);
				smoothrender.re_init_line(cardinal);
			}
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow* window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	GLuint storeDataInAttributeList(int attributeNumber, vector<float>* data) {
		GLuint vboID;
		glGenBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, data[0].size() * sizeof(float), data[0].data(), GL_STATIC_DRAW);
		glVertexAttribPointer(attributeNumber, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(attributeNumber);
		return vboID;
	}

	std::vector<std::string> split(const std::string str, char delim)
	{
		std::vector<std::string> result;
		std::istringstream ss{ str };
		std::string token;
		while (std::getline(ss, token, delim)) {
			if (!token.empty()) {
				result.push_back(token);
			}
		}
		return result;
	}

	struct WordTiming
	{
		string word;
		float arrivalTime;
	};

	vector<WordTiming> GenerateWordsAndTimes(string filename)
	{
		vector < WordTiming> lyrics;
		fstream file;
		file.open(filename, ios::in);
		if (file.is_open())
		{
			string line;
			while (getline(file, line))
			{
				vector<string> values = split(line, ' ');
				if (values.size() < 2)
				{
					break;
				}
				WordTiming w;
				w.word = values[0];
				w.arrivalTime = stof(values[1]);
				lyrics.push_back(w);
			}
			file.close();
		}
		return lyrics;
	}

	vector<WordTiming> song_lyrics;

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string& resourceDirectory)
	{
		song_lyrics = GenerateWordsAndTimes(resourceDirectory + "/new_times_in_seconds.txt");
		face = make_shared<Shape>();
		o = make_shared<Shape>();
		a = make_shared<Shape>();
		ee = make_shared<Shape>();
		f = make_shared<Shape>();
		l = make_shared<Shape>();
		m = make_shared<Shape>();
		t_face = make_shared<Shape>();
		i_face = make_shared<Shape>();


		face->loadMesh(resourceDirectory + "/femaleFace/neutral.obj");
		face->resize();
		face->init();

		o->loadMesh(resourceDirectory + "/femaleFace/o.obj");
		o->resize();
		o->init();

		a->loadMesh(resourceDirectory + "/femaleFace/a.obj");
		a->resize();
		a->init();

		ee->loadMesh(resourceDirectory + "/femaleFace/e.obj");
		ee->resize();
		ee->init();

		f->loadMesh(resourceDirectory + "/femaleFace/f.obj");
		f->resize();
		f->init();

		l->loadMesh(resourceDirectory + "/femaleFace/l.obj");
		l->resize();
		l->init();

		m->loadMesh(resourceDirectory + "/femaleFace/m.obj");
		m->resize();
		m->init();

		t_face->loadMesh(resourceDirectory + "/femaleFace/t.obj");
		t_face->resize();
		t_face->init();

		i_face->loadMesh(resourceDirectory + "/femaleFace/i.obj");
		i_face->resize();
		i_face->init();


		string musicDir = resourceDirectory + "/goodnightfade.mp3";
		char* musicName = new char[musicDir.length() + 1];
		strcpy(musicName, musicDir.c_str());

		musicID = music.init_music(musicName);
		delete[] musicName;

		// add all the positions to the attributes of the inital "face" -> face

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		storeDataInAttributeList(0, face->posBuf);
		storeDataInAttributeList(1, face->norBuf);
		GLuint vboID;
		glGenBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, face->texBuf[0].size() * sizeof(float), face->texBuf[0].data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(2);

		storeDataInAttributeList(3, o->posBuf);
		storeDataInAttributeList(4, a->posBuf);
		storeDataInAttributeList(5, ee->posBuf);
		storeDataInAttributeList(6, f->posBuf);
		storeDataInAttributeList(7, l->posBuf);
		storeDataInAttributeList(8, m->posBuf);
		storeDataInAttributeList(9, i_face->posBuf);
		storeDataInAttributeList(10, t_face->posBuf);

		glGenBuffers(1, &IndexBufferIDBox);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, face->eleBuf[0].size() * sizeof(unsigned int),
			face->eleBuf[0].data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// initialize Framebuffers for bloom
		// initialize the frame buffer object
		glGenFramebuffers(1, &bFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, bFBO);


		int screenW, screenH;
		glfwGetFramebufferSize(windowManager->getHandle(), &screenW, &screenH);

		glGenTextures(2, colorBuffers);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA16F, screenW, screenH, 0, GL_RGBA, GL_FLOAT, NULL
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach texture to framebuffer
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
			);
		}
		// attach depth buffer to render buffer
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, screenW, screenH);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		// specify the color buffers to draw into
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;

		// create ping pong fbo for 2 pass gaussian blur
		glGenFramebuffers(2, pingpongFBO);
		glGenTextures(2, pingpongBuffer);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA16F, screenW, screenH, 0, GL_RGBA, GL_FLOAT, NULL
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
			);
		}

		// specify which locations to use in the final shader
		GLuint Tex0Location = glGetUniformLocation(fBloomP->pid, "scene");
		GLuint Tex1Location = glGetUniformLocation(fBloomP->pid, "bloomBlur");
		glUseProgram(fBloomP->pid);
		glUniform1i(Tex0Location, 0);
		glUniform1i(Tex1Location, 1);

		// load lyric textures:
		int width, height, channels;
		char filepath[1000];
		string str = resourceDirectory + "/lyrics_flipped.png";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLuint Tex2Location = glGetUniformLocation(fBloomP->pid, "lyrics");
		glUseProgram(fBloomP->pid);
		glUniform1i(Tex2Location, 2);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();
		int width, height;
		glfwSetCursorPosCallback(windowManager->getHandle(), mouse_curs_callback);
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		faceprog = std::make_shared<Program>();
		faceprog->setVerbose(true);
		faceprog->setShaderNames(resourceDirectory + "/plane_vertex.glsl", resourceDirectory + "/plane_frag.glsl");
		if (!faceprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		faceprog->addUniform("P");
		faceprog->addUniform("V");
		faceprog->addUniform("M");
		faceprog->addUniform("campos");
		faceprog->addUniform("currentChar");
		faceprog->addUniform("nextChar");
		faceprog->addUniform("t");
		faceprog->addAttribute("neutral");
		faceprog->addAttribute("vertNor");
		faceprog->addAttribute("vertTex");
		faceprog->addAttribute("o");
		faceprog->addAttribute("a");
		faceprog->addAttribute("e");
		faceprog->addAttribute("f");
		faceprog->addAttribute("l");
		faceprog->addAttribute("m");
		faceprog->addAttribute("i");
		faceprog->addAttribute("t_face");

		bloomP = std::make_shared<Program>();
		bloomP->setVerbose(true);
		bloomP->setShaderNames(resourceDirectory + "/bloom_vertex.glsl", resourceDirectory + "/bloom_fragment.glsl");
		if (!bloomP->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		bloomP->addAttribute("vertPos");
		bloomP->addAttribute("vertTex");
		bloomP->addUniform("horizontal");
		//bloomP->addUniform("weight");

		fBloomP = std::make_shared<Program>();
		fBloomP->setVerbose(true);
		fBloomP->setShaderNames(resourceDirectory + "/final_vertex.glsl", resourceDirectory + "/final_fragment.glsl");
		if (!fBloomP->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		fBloomP->addAttribute("vertPos");
		fBloomP->addAttribute("vertTex");
		fBloomP->addUniform("exposure");
		fBloomP->addUniform("bloom");
		fBloomP->addUniform("glTime");
	}

	void renderQuad()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	void renderBloom(float timeSinceStart)
	{
		// blur fragments
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 30;
		// use the blur program to draw the scene
		bloomP->bind();
		for (unsigned int i = 0; i < amount; i++)
		{
			// bind the appropriate buffer to draw to
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			glUniform1i(bloomP->getUniform("horizontal"), horizontal);
			// first time rendering -> use extracted colors, otherwise use from previous step.
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(
				GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffer[!horizontal]
			);
			renderQuad();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		bloomP->unbind();
		// set render output to screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// mix the two frambuffers and send it out to the screen
		float exposure = 1.0f;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fBloomP->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		//glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		//glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		glActiveTexture(GL_TEXTURE1);
		//BindTexture(GL_TEXTURE_2D, colorBuffers[1]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(fBloomP->getUniform("bloom"), true);
		glUniform1f(fBloomP->getUniform("exposure"), exposure);
		glUniform1f(fBloomP->getUniform("glTime"), timeSinceStart);
		renderQuad();
		fBloomP->unbind();
	}

	struct MouthData
	{
		char curr, next;
		float t;		// amount to blend between the two.
	};

	/*
	 * Returns a vector of 2 chars, first one is the current letter,
	 * the second is the one to interpolate to
	 */
	MouthData GetCurrentLetter(float startTime, float w)
	{
		static int prevIndex = -1;
		float songtime = w - startTime;
		int index = -1;
		static char lastChar = '.';
		MouthData m;
		m.curr = m.next = '.';
		m.t = 0.0;

		for (int i = 0; i < song_lyrics.size() - 1; i++)
		{
			if (songtime <= song_lyrics[i + 1].arrivalTime && songtime >= song_lyrics[i].arrivalTime)
			{
				index = i;
				break;
			}
		}

		if (index != -1)
		{
			// subdivide the current word by the amount of letters in it
			string curr_word = song_lyrics[index].word;
			float word_duration = song_lyrics[index + 1].arrivalTime - song_lyrics[index].arrivalTime;
			float letter_duration = word_duration / curr_word.size();
			float word_progress = ((songtime - song_lyrics[index].arrivalTime) / letter_duration);
			int charIndex = (int)word_progress;
			assert(charIndex < curr_word.size());
			char currChar = curr_word[charIndex];
			char nextChar;
			if (charIndex == curr_word.size() - 1)
			{
				nextChar = song_lyrics[index + 1].word[0];
			}
			else
			{
				nextChar = curr_word[charIndex + 1];
			}
			float t = (songtime - song_lyrics[index].arrivalTime) - (charIndex * letter_duration);
			// assert(t <= 1);
			m.curr = currChar;
			m.next = nextChar;
			m.t = t;
		}
		if (m.curr != lastChar)
		{
			cout << lastChar;
			lastChar = m.curr;
		}
		if (index != -1 && index != prevIndex) {
			//cout << song_lyrics[index].word << endl;
			cout << endl;
			prevIndex = index;
		}

		// we have the current word, and we can get the next word

		return m;
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		static int hasRunMusic = 0;
		static int hasInitializedStartTime = 0;
		MouthData m;
		m.curr = m.next = '.';
		m.t = 0.0;
		static float startTime = 0.0;

		double frametime = get_last_elapsed_time();
		static float w = 0.0f;
		w += frametime;
		
		if (!hasRunMusic)
		{
			hasRunMusic++;
			music.play(musicID);
		}
		else
		{
			if (!hasInitializedStartTime)
			{
				startTime = glfwGetTime();
				w = glfwGetTime();
				hasInitializedStartTime++;
			}
			m = GetCurrentLetter(startTime, w);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		vec2 mouseNorm = vec2(lastX / width, lastY / height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, bFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		// Create the matrix stacks - please leave these alone for now
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		// Draw the face using GLSL.
		glm::mat4 Trans = glm::translate(glm::mat4(1.0f), vec3(0, 0, -3));
		glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
		sangle = 3.1415926;
		glm::mat4 RotY = glm::rotate(glm::mat4(1.0f), 0.f, vec3(0, 1, 0));

		M = Trans * RotY * Scale;

		faceprog->bind();
		glUniformMatrix4fv(faceprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(faceprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(faceprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(faceprog->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1i(faceprog->getUniform("currentChar"), (int)(m.curr - 'a'));
		glUniform1i(faceprog->getUniform("nextChar"), (int)(m.next - 'a'));
		glUniform1f(faceprog->getUniform("t"), m.t);
		// glUniform2fv(faceprog->getUniform("mouseNorm"), 1, &mouseNorm[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);

		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, (int)face->eleBuf[0].size(), GL_UNSIGNED_INT, (const void*)0);
		glBindVertexArray(0);

		faceprog->unbind();
		renderBloom(w - startTime);
		if (w - startTime > 63.0f)
		{
			glfwSetWindowShouldClose(windowManager->getHandle(), GL_TRUE);
		}
	}

};
//******************************************************************************************
int main(int argc, char** argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application* application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager* windowManager = new WindowManager();
	windowManager->init(640, 360);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
		// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}