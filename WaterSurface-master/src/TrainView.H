/************************************************************************
     File:        TrainView.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;
class Model;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#include"TrainWindow.H"
#pragma warning(pop)
#include<string>
#include<iostream>

#include <AL/alut.h>
//#include <AL/al.h>
//#include <AL/alc.h>

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"

class TrainView : public Fl_Gl_Window
{
	public:
		// note that we keep the "standard widget" constructor arguments
		TrainView(int x, int y, int w, int h, const char* l = 0);

		// overrides of important window things
		virtual int handle(int);
		virtual void draw();

		// all of the actual drawing happens in this routine
		// it has to be encapsulated, since we draw differently if
		// we're drawing shadows (no colors, for example)
		void drawStuff(bool doingShadows=false);

		// setup the projection - assuming that the projection stack has been
		// cleared for you
		void setProjection();

		// Reset the Arc ball control
		void resetArcball();

		// pick a point (for when the mouse goes down)
		void doPick();

		//set ubo
		void setUBO();
		//train
		Pnt3f GMT(Pnt3f p1, Pnt3f p2, Pnt3f p3, Pnt3f p4, float mode, float t);
		void drawTrack(TrainView*, bool);
		void drawTrain(TrainView*, bool);
		void trainCamView(TrainView*, float);
	public:
		ArcBallCam		arcball;			// keep an ArcBall for the UI
		int				selectedCube;  // simple - just remember which cube is selected

		TrainWindow*	tw;				// The parent of this display window
		CTrack*			m_pTrack;		// The track of the entire scene
		//Train
		float physical = 0.0f;
		float DIVIDE_LINE = 1000.0f;
		float t_time = 0.0f;

		float t_i = 0.0f;
		float t_t = 0.0f;
		float Path_Total = 0.0f;

		float Sleeper_Length = 3.0f;
		float Sleeper_Interval = 6.0f;
		float Sleeper_Width = 3.0f;

		float Train_Forward = 10.0f;
		float Train_Height = 5.0f;
		float Train_Width = 2.0f;
		float tense = 0.5f;
		int car_quantity = 2;
		bool s = false;
		

		//Wave
		UBO* commom_matrices= nullptr;
		
		//OpenAL
		glm::vec3 source_pos;
		glm::vec3 listener_pos;
		ALCdevice* device	= nullptr;
		ALCcontext* context = nullptr;
		ALuint source;
		ALuint buffer;


		GLfloat View[16];
		GLfloat Projection[16];

		Model* wave_model = nullptr;
		Model* tile_model = nullptr;

		Texture2D* tile_texture = nullptr;
		Texture2D* particle_texture = nullptr;

		float height_index = 0;
		bool first = true;
		bool skybox_load = true;
		std::vector<Texture2D> height_id;

		//Shader
		Shader* particle_shader = nullptr;
		Shader* wave_shader = nullptr;
		Shader* tile = nullptr;
		Shader* skybox = nullptr;
		Shader* screenShader = nullptr;
		Shader* interactive_frame = nullptr;

		GLuint particleVAO;
		GLuint cubemapTexture;
		GLuint skyboxVAO, skyboxVBO;
		//frame buffer
		unsigned int framebuffer;
		unsigned int textureColorbuffer;
		unsigned int quadVAO, quadVBO;

		unsigned int framebuffer1;
		unsigned int textureColorbuffer1;
		unsigned int quadVAO1, quadVBO1;

		glm::vec2 uv_center;
		int uv_t = 0;

		float scale = 20;

		//particle
		int FindUnusedParticle();
		void SortParticles();

		struct Particle {
			glm::vec3 pos, speed;
			unsigned char r, g, b, a; // Color
			float size, angle, weight;
			float life; // Remaining life of the particle. if <0 : dead and unused.
			float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

			bool operator<(const Particle& that) const {
				// Sort in reverse order : far particles drawn first.
				return this->cameradistance > that.cameradistance;
			}
		};

		const int MaxParticles = 100000;
		Particle ParticlesContainer[100000];
		int LastUsedParticle = 0;
	    GLfloat* g_particule_position_size_data;
		GLubyte* g_particule_color_data;
		GLuint particles_position_buffer;
		GLuint particles_color_buffer;
		GLuint billboard_vertex_buffer;
		double lastTime;
		double delta;
};		