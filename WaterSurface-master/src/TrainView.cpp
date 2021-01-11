﻿/************************************************************************
     File:        TrainView.cpp

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

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glu.h>


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"

#define STB_IMAGE_IMPLEMENTATION
//#define heightmap
#define particlee
#include"RenderUtilities/model.h"
//#define DEBUG

void TrainView::firework_init() {
	fire_pos.clear();
	fire_angle.clear();
	fire_pos.push_back(glm::mat4(1));
	fire_angle.push_back(90);
	firework_speed = 1.0;
	firework_radius = 1;
	firework_circle = 1000;
}
void TrainView::firework_update() {
	srand(clock());
	int radius = 30;
	float offset = 2.5f;
	if (fire_pos[0][3][1] > 150) {
		firework_radius++;
		if ((int)firework_radius % 2 == 0) {
			fire_pos.clear();
			fire_angle.clear();
		}
		
		for (int j = 0; j < firework_circle; j++) {
			glm::mat4 model = glm::mat4(1.0f);
			float angle = (float)j / (float)firework_circle * 360.0f;
			float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float x = cos(angle*3.1415926/180) * firework_radius + displacement;
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

			float y = fire_pos[0][3][1] + sin(angle * 3.1415926 / 180) * firework_radius;

			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float z = 0;
			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. scale: scale between 0.05 and 0.25f
			float scale = (rand() % 50) / 300.0f + 0.05;
			model = glm::scale(model, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			float rotAngle = (rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			fire_pos.push_back(model);
			fire_angle.push_back(angle);
		}
		if (firework_radius > radius) {
			firework_over = true;
		}
	}
	else {
		firework_speed *= 1.03;
		fire_pos[0][3][1] += firework_speed;
		fire_angle[0] = rand() % 180;
	}
}

void TrainView::snow_update() {
	snow_amount = tw->buildingCounter->value() * 10000;
	for (int i = 0; i < snow_amount; i++) {
		if (snow_pos[i].y > 0) {
			srand(clock());
			float y = 0.5;
			snow_pos[i].y -= y;
		}
		else {
			snow_pos.erase(snow_pos.begin() + i);
			i--;
			float x = rand() % 2000 - 1000;
			float z = rand() % 2000 - 1000;
			float y = rand() % 1000 + 200;
			snow_pos.push_back(glm::vec3(x, y, z));
		}
		glm::mat4 snow_transfer(1);
		snow_transfer = glm::translate(snow_transfer, snow_pos[i]);
		modelMatrices[i] = snow_transfer;
	}
}

void normalize(GLfloat* v)
{
	GLfloat d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= d; v[1] /= d; v[2] /= d;
}

void TrainView::Draw()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	particle_shader->Use();
	for (Particle particle : this->particles)
	{
		if (particle.Life > 0.0f)
		{
			glUniformMatrix4fv(glGetUniformLocation(particle_shader->Program, "model_matrix"), 1, GL_FALSE, &particle.Module[0][0]);

			glUniform1f(glGetUniformLocation(particle_shader->Program, "scale"), particle.Scale);
			glUniform4f(glGetUniformLocation(particle_shader->Program, "color"), particle.Color.r, particle.Color.g, particle.Color.b, particle.Color.a);
			glBindVertexArray(this->particleVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
	}
	// Don't forget to reset to default blending mode
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void TrainView::RespawnParticle(Particle& particle)
{
	particle.Scale_randon = (float)((rand() % 1000) - 500) / 1000.0f;
	particle.X_random = (float)((rand() % 100) - 50) / 15.0f;
	particle.Y_random = (float)((rand() % 100) - 50) / 15.0f;
	particle.Z_random = (float)((rand() % 100) - 50) / 15.0f;

	GLfloat rColor = 0.286;

	particle.trainPos = trainPos;
	particle.Module = glm::mat4(1.0f);
	particle.Module = glm::translate(particle.Module, glm::vec3(trainPos.x+ particle.X_random*0.5, trainPos.y + 10+ particle.Y_random*0.5 , trainPos.z+ particle.Z_random*0.5));
	particle.Module = glm::scale(particle.Module, glm::vec3(particle.Scale_randon, particle.Scale_randon, particle.Scale_randon));

	particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
	particle.Life = 1.0f;

}

GLuint TrainView::FirstUnusedParticle()
{
	// Search from last used particle, this will usually return almost instantly
	for (GLuint i = lastUsedParticle; i < nr_particles; ++i) {
		if (particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// Otherwise, do a linear search
	for (GLuint i = 0; i < lastUsedParticle; ++i) {
		if (particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// Override first particle if all others are alive
	lastUsedParticle = 0;
	return 0;
}

//My function
Pnt3f TrainView::GMT(Pnt3f p1, Pnt3f p2, Pnt3f p3, Pnt3f p4, float mode, float t) {
	glm::mat4x4 G = {
		{p1.x,p2.x,p3.x,p4.x},
		{p1.y,p2.y,p3.y,p4.y},
		{p1.z,p2.z,p3.z,p4.z},
		{1,1,1,1}
	};
	G = glm::transpose(G);
	glm::mat4x4 M;
	if (mode == 1) {
		M = { 0, 0, 0, 0,
			0, 0, -1, 1,
			0, 0, 1, 0,
			0, 0, 0, 0 };
	}
	else if (mode == 2) {
		M = {
				{-1.0f,2.0f,-1.0f,0.0f},
				{3.0f,-5.0f,0.0f,2.0f },
				{-3.0f,4.0f,1.0f,0.0f},
				{1.0f,-1.0f,0.0f,0.0f}
		};
		M /= 2.0f;
	}
	else if (mode == 3) {
		M = {
			{-1.0f,3.0f,-3.0f,1.0f},
			{3.0f,-6.0f,0.0f,4.0f},
			{-3.0f,3.0f,3.0f,1.0f},
			{1.0f,0.0f,0.0f,0.0f}
		};
		M /= 6.0f;
	}
	M = glm::transpose(M);
	glm::vec4 T = { pow(t,3),pow(t,2),pow(t,1),pow(t,0) };
	glm::vec4 result = G * M * T;
	return Pnt3f(result[0], result[1], result[2]);
}
void glVertex3f_Simplify(Pnt3f q0) {
	glVertex3f(q0.x, q0.y, q0.z);
}
void DrawSleeper(Pnt3f qt0, Pnt3f qt1, Pnt3f cross_t, Pnt3f orient_t, bool doingShadows) {
	//畫鐵軌的線
	if (!doingShadows) {
		glColor3ub(0, 0, 0);
	}
	glLineWidth(2);
	glBegin(GL_LINES);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glVertex3f_Simplify(qt1 + cross_t);
	glVertex3f_Simplify(qt1 - cross_t);

	glVertex3f_Simplify(qt0 + cross_t - orient_t);
	glVertex3f_Simplify(qt0 - cross_t - orient_t);
	glVertex3f_Simplify(qt1 + cross_t - orient_t);
	glVertex3f_Simplify(qt1 - cross_t - orient_t);

	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 + cross_t - orient_t);
	glVertex3f_Simplify(qt1 + cross_t);
	glVertex3f_Simplify(qt1 + cross_t - orient_t);

	glVertex3f_Simplify(qt0 - cross_t);
	glVertex3f_Simplify(qt0 - cross_t - orient_t);
	glVertex3f_Simplify(qt1 - cross_t);
	glVertex3f_Simplify(qt1 - cross_t - orient_t);


	glVertex3f_Simplify(qt0 - cross_t - orient_t);
	glVertex3f_Simplify(qt1 - cross_t - orient_t);


	glVertex3f_Simplify(qt0 + cross_t - orient_t);
	glVertex3f_Simplify(qt1 + cross_t - orient_t);
	glEnd();

	//畫鐵軌
	if (!doingShadows) {
		glColor3ub(101, 50, 0);
	}
	glNormal3f(orient_t.x, orient_t.y, orient_t.z);
	glBegin(GL_QUADS);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt1 + cross_t);
	glVertex3f_Simplify(qt1 - cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glEnd();

	glNormal3f(-orient_t.x, -orient_t.y, -orient_t.z);
	glBegin(GL_QUADS);
	glVertex3f_Simplify(qt0 - orient_t + cross_t);
	glVertex3f_Simplify(qt1 - orient_t + cross_t);
	glVertex3f_Simplify(qt1 - orient_t - cross_t);
	glVertex3f_Simplify(qt0 - orient_t - cross_t);
	glEnd();

	glNormal3f(-(qt1 - qt0).x, -(qt1 - qt0).y, -(qt1 - qt0).z);
	glBegin(GL_QUADS);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glVertex3f_Simplify(qt0 - orient_t - cross_t);
	glVertex3f_Simplify(qt0 - orient_t + cross_t);
	glEnd();

	glNormal3f((qt1 - qt0).x, (qt1 - qt0).y, (qt1 - qt0).z);
	glBegin(GL_QUADS);
	glVertex3f_Simplify(qt1 + cross_t);
	glVertex3f_Simplify(qt1 - cross_t);
	glVertex3f_Simplify(qt1 - orient_t - cross_t);
	glVertex3f_Simplify(qt1 - orient_t + cross_t);
	glEnd();

	glNormal3f(-cross_t.x, -cross_t.y, -cross_t.z);
	glBegin(GL_QUADS);
	glVertex3f_Simplify(qt1 - cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glVertex3f_Simplify(qt0 - orient_t - cross_t);
	glVertex3f_Simplify(qt1 - orient_t - cross_t);
	glEnd();

	glNormal3f(cross_t.x, cross_t.y, cross_t.z);
	glBegin(GL_QUADS);
	glVertex3f_Simplify(qt1 + cross_t);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 - orient_t + cross_t);
	glVertex3f_Simplify(qt1 - orient_t + cross_t);
	glEnd();
}
void DrawPillar(Pnt3f qt0, Pnt3f qt1, Pnt3f cross_t, Pnt3f orient_t, bool doingShadows) {
	/*柱子*/
	if (!doingShadows) {
		glColor3ub(64, 17, 140);
	}
	//下
	glBegin(GL_QUADS);
	glVertex3f((qt0 + cross_t).x, 0.1, (qt0 + cross_t).z);
	glVertex3f((qt1 + cross_t).x, 0.1, (qt1 + cross_t).z);
	glVertex3f((qt1 - cross_t).x, 0.1, (qt1 - cross_t).z);
	glVertex3f((qt0 - cross_t).x, 0.1, (qt0 - cross_t).z);
	glEnd();
	//右
	glNormal3f(cross_t.x, cross_t.y, cross_t.z);
	glBegin(GL_QUADS);
	glVertex3f((qt0 + cross_t).x, (qt0 + cross_t).y, (qt0 + cross_t).z);
	glVertex3f((qt1 + cross_t).x, (qt1 + cross_t).y, (qt1 + cross_t).z);
	glVertex3f((qt1 + cross_t).x, 0.1, (qt1 + cross_t).z);
	glVertex3f((qt0 + cross_t).x, 0.1, (qt0 + cross_t).z);
	glEnd();
	//左
	glNormal3f(-cross_t.x, -cross_t.y, -cross_t.z);
	glBegin(GL_QUADS);
	glVertex3f((qt0 - cross_t).x, (qt0 - cross_t).y, (qt0 - cross_t).z);
	glVertex3f((qt1 - cross_t).x, (qt1 - cross_t).y, (qt1 - cross_t).z);
	glVertex3f((qt1 - cross_t).x, 0.1, (qt1 - cross_t).z);
	glVertex3f((qt0 - cross_t).x, 0.1, (qt0 - cross_t).z);
	glEnd();
	//前
	glNormal3f((qt1 - qt0).x, (qt1 - qt0).y, (qt1 - qt0).z);
	glBegin(GL_QUADS);
	glVertex3f((qt1 - cross_t).x, (qt1 - cross_t).y, (qt1 - cross_t).z);
	glVertex3f((qt1 + cross_t).x, (qt1 - cross_t).y, (qt1 + cross_t).z);
	glVertex3f((qt1 + cross_t).x, 0.1, (qt1 + cross_t).z);
	glVertex3f((qt1 - cross_t).x, 0.1, (qt1 - cross_t).z);
	glEnd();
	//後
	glNormal3f(-(qt1 - qt0).x, -(qt1 - qt0).y, -(qt1 - qt0).z);
	glBegin(GL_QUADS);
	glVertex3f((qt0 - cross_t).x, (qt0 - cross_t).y, (qt0 - cross_t).z);
	glVertex3f((qt0 + cross_t).x, (qt0 - cross_t).y, (qt0 + cross_t).z);
	glVertex3f((qt0 + cross_t).x, 0.1, (qt0 + cross_t).z);
	glVertex3f((qt0 - cross_t).x, 0.1, (qt0 - cross_t).z);
	glEnd();
	/*柱子*/
}
void DrawTrain(Pnt3f qt0, Pnt3f cross_t, Pnt3f up, Pnt3f forward, bool doingShadows) {
	if (!doingShadows) {
		glColor3ub(255, 255, 255);
	}
	//上
	/*glBegin(GL_QUADS);
	glNormal3f(up.x, up.y, up.z);
	glVertex3f_Simplify(qt0 + cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t + up);
	glVertex3f_Simplify(qt0 + forward - cross_t + up);
	glVertex3f_Simplify(qt0 - cross_t + up);
	glEnd();*/
	//下
	glBegin(GL_QUADS);
	glNormal3f(-up.x, -up.y, -up.z);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 + forward + cross_t);
	glVertex3f_Simplify(qt0 + forward - cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glEnd();
	//前
	glBegin(GL_QUADS);
	glNormal3f(forward.x, forward.y, forward.z);
	glVertex3f_Simplify(qt0 + forward - cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t);
	glVertex3f_Simplify(qt0 + forward - cross_t);
	glEnd();
	//後
	glBegin(GL_QUADS);
	glNormal3f(-forward.x, -forward.y, -forward.z);
	glVertex3f_Simplify(qt0 - cross_t + up);
	glVertex3f_Simplify(qt0 + cross_t + up);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glEnd();
	//左
	glBegin(GL_QUADS);
	glNormal3f(-cross_t.x, -cross_t.y, -cross_t.z);
	glVertex3f_Simplify(qt0 + forward - cross_t + up);
	glVertex3f_Simplify(qt0 + forward - cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glVertex3f_Simplify(qt0 - cross_t + up);
	glEnd();
	//右
	glBegin(GL_QUADS);
	glNormal3f(cross_t.x, cross_t.y, cross_t.z);
	glVertex3f_Simplify(qt0 + forward + cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 + cross_t + up);
	glEnd();
	if (!doingShadows) {
		glColor3ub(0, 0, 0);
	}
	//後輪
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 - cross_t * 1.2);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 0.1);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 0.2 + up * 0.2);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 0.1 + up * 0.4);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + up * 0.4);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 - forward * 0.1 + up * 0.2);
	glEnd();
	glFlush();
	//前輪
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 0.9);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 1);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 1.1 + up * 0.2);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 1 + up * 0.4);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + up * 0.4 + forward * 0.9);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 0.8 + up * 0.2);
	glEnd();
	glFlush();
	//前輪
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 + cross_t * 1.2);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 0.1);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 0.2 + up * 0.2);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 0.1 + up * 0.4);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + up * 0.4);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 - forward * 0.1 + up * 0.2);
	glEnd();
	glFlush();
	//前輪                  
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 0.9);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 1);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 1.1 + up * 0.2);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 1 + up * 0.4);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + up * 0.4 + forward * 0.9);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 0.8 + up * 0.2);
	glEnd();
}
void DrawTrainHead(Pnt3f qt0, Pnt3f cross_t, Pnt3f up, Pnt3f forward, bool doingShadows) {
	if (!doingShadows) {
		glColor3ub(0, 30, 150);
	}
	forward = forward * 1.5;
	//上
	glBegin(GL_QUADS);
	glNormal3f(up.x, up.y, up.z);
	glVertex3f_Simplify(qt0 + cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t + up);
	glVertex3f_Simplify(qt0 + forward - cross_t + up);
	glVertex3f_Simplify(qt0 - cross_t + up);
	glEnd();
	//下
	glBegin(GL_QUADS);
	glNormal3f(-up.x, -up.y, -up.z);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 + forward + cross_t);
	glVertex3f_Simplify(qt0 + forward - cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glEnd();
	//前
	glBegin(GL_QUADS);
	glNormal3f(forward.x, forward.y, forward.z);
	glVertex3f_Simplify(qt0 + forward - cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t);
	glVertex3f_Simplify(qt0 + forward - cross_t);
	glEnd();
	//後
	glBegin(GL_QUADS);
	glNormal3f(-forward.x, -forward.y, -forward.z);
	glVertex3f_Simplify(qt0 - cross_t + up);
	glVertex3f_Simplify(qt0 + cross_t + up);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glEnd();
	//左
	glBegin(GL_QUADS);
	glNormal3f(-cross_t.x, -cross_t.y, -cross_t.z);
	glVertex3f_Simplify(qt0 + forward - cross_t + up);
	glVertex3f_Simplify(qt0 + forward - cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glVertex3f_Simplify(qt0 - cross_t + up);
	glEnd();
	//右
	glBegin(GL_QUADS);
	glNormal3f(cross_t.x, cross_t.y, cross_t.z);
	glVertex3f_Simplify(qt0 + forward + cross_t + up);
	glVertex3f_Simplify(qt0 + forward + cross_t);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 + cross_t + up);
	glEnd();
	/*後面的部分*/
	float height = 1.8f;
	float length = 0.4f;
	cross_t = cross_t * 1.2;
	//上
	glBegin(GL_QUADS);
	glNormal3f(up.x, up.y, up.z);
	glVertex3f_Simplify(qt0 + cross_t + up * height);
	glVertex3f_Simplify(qt0 + forward * length + cross_t + up * height);
	glVertex3f_Simplify(qt0 + forward * length - cross_t + up * height);
	glVertex3f_Simplify(qt0 - cross_t + up * height);
	glEnd();
	//後
	glBegin(GL_QUADS);
	glNormal3f(-forward.x, -forward.y, -forward.z);
	glVertex3f_Simplify(qt0 - cross_t + up * height);
	glVertex3f_Simplify(qt0 + cross_t + up * height);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glEnd();
	//前
	glBegin(GL_QUADS);
	glNormal3f(forward.x, forward.y, forward.z);
	glVertex3f_Simplify(qt0 + forward * length - cross_t + up * height);
	glVertex3f_Simplify(qt0 + forward * length + cross_t + up * height);
	glVertex3f_Simplify(qt0 + forward * length + cross_t);
	glVertex3f_Simplify(qt0 + forward * length - cross_t);
	glEnd();
	//左
	glBegin(GL_QUADS);
	glNormal3f(-cross_t.x, -cross_t.y, -cross_t.z);
	glVertex3f_Simplify(qt0 + forward * length - cross_t + up * height);
	glVertex3f_Simplify(qt0 + forward * length - cross_t);
	glVertex3f_Simplify(qt0 - cross_t);
	glVertex3f_Simplify(qt0 - cross_t + up * height);
	glEnd();
	//右
	glBegin(GL_QUADS);
	glNormal3f(-cross_t.x, -cross_t.y, -cross_t.z);
	glVertex3f_Simplify(qt0 + forward * length + cross_t + up * height);
	glVertex3f_Simplify(qt0 + forward * length + cross_t);
	glVertex3f_Simplify(qt0 + cross_t);
	glVertex3f_Simplify(qt0 + cross_t + up * height);
	glEnd();
	cross_t = cross_t / 1.2;
	forward = forward / 1.5;
	if (!doingShadows) {
		glColor3ub(0, 0, 0);
	}
	//後輪
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 - cross_t * 1.4);
	glVertex3f_Simplify(qt0 - cross_t * 1.4 + forward * 0.1);
	glVertex3f_Simplify(qt0 - cross_t * 1.4 + forward * 0.2 + up * 0.2);
	glVertex3f_Simplify(qt0 - cross_t * 1.4 + forward * 0.1 + up * 0.4);
	glVertex3f_Simplify(qt0 - cross_t * 1.4 + up * 0.4);
	glVertex3f_Simplify(qt0 - cross_t * 1.4 - forward * 0.1 + up * 0.2);
	glEnd();
	glFlush();
	//前輪
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 0.9);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 1);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 1.1 + up * 0.2);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 1 + up * 0.4);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + up * 0.4 + forward * 0.9);
	glVertex3f_Simplify(qt0 - cross_t * 1.2 + forward * 0.8 + up * 0.2);
	glEnd();
	glFlush();
	//前輪
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 + cross_t * 1.4);
	glVertex3f_Simplify(qt0 + cross_t * 1.4 + forward * 0.1);
	glVertex3f_Simplify(qt0 + cross_t * 1.4 + forward * 0.2 + up * 0.2);
	glVertex3f_Simplify(qt0 + cross_t * 1.4 + forward * 0.1 + up * 0.4);
	glVertex3f_Simplify(qt0 + cross_t * 1.4 + up * 0.4);
	glVertex3f_Simplify(qt0 + cross_t * 1.4 - forward * 0.1 + up * 0.2);
	glEnd();
	glFlush();
	//前輪                  
	glBegin(GL_POLYGON);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 0.9);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 1);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 1.1 + up * 0.2);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 1 + up * 0.4);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + up * 0.4 + forward * 0.9);
	glVertex3f_Simplify(qt0 + cross_t * 1.2 + forward * 0.8 + up * 0.2);
	glEnd();
}
unsigned int loadCubemap(vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return textureID;
}
//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l) 
	: Fl_Gl_Window(x,y,w,h,l)
//========================================================================
{
	mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );

	resetArcball();
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event)) 
			return 1;

	// remember what button was used
	static int last_push;

	switch(event) {
		// Mouse button being pushed event
		case FL_PUSH:
			last_push = Fl::event_button();
			// if the left button be pushed is left mouse button
			if (last_push == FL_LEFT_MOUSE  ) {
				doPick();
				damage(1);
				return 1;
			};
			break;

	   // Mouse button release event
		case FL_RELEASE: // button release
			damage(1);
			last_push = 0;
			return 1;

		// Mouse button drag event
		case FL_DRAG:

			// Compute the new control point position
			if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
				ControlPoint* cp = &m_pTrack->points[selectedCube];

				double r1x, r1y, r1z, r2x, r2y, r2z;
				getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

				double rx, ry, rz;
				mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z, 
								static_cast<double>(cp->pos.x), 
								static_cast<double>(cp->pos.y),
								static_cast<double>(cp->pos.z),
								rx, ry, rz,
								(Fl::event_state() & FL_CTRL) != 0);

				cp->pos.x = (float) rx;
				cp->pos.y = (float) ry;
				cp->pos.z = (float) rz;
				damage(1);
			}
			break;

		// in order to get keyboard events, we need to accept focus
		case FL_FOCUS:
			return 1;

		// every time the mouse enters this window, aggressively take focus
		case FL_ENTER:	
			focus(this);
			break;

		case FL_KEYBOARD:
		 		int k = Fl::event_key();
				int ks = Fl::event_state();
				if (k == 'p') {
					// Print out the selected control point information
					if (selectedCube >= 0) 
						printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
								 selectedCube,
								 m_pTrack->points[selectedCube].pos.x,
								 m_pTrack->points[selectedCube].pos.y,
								 m_pTrack->points[selectedCube].pos.z,
								 m_pTrack->points[selectedCube].orient.x,
								 m_pTrack->points[selectedCube].orient.y,
								 m_pTrack->points[selectedCube].orient.z);
					else
						printf("Nothing Selected\n");

					return 1;
				};
				break;
	}

	return Fl_Gl_Window::handle(event);
}

//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	//if (gladLoadGL())
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		if (!this->particle_shader) {
			this->particle_shader = new
				Shader(
					"../WaterSurface-master/src/shaders/particle.vert",
					nullptr, nullptr, nullptr,
					"../WaterSurface-master/src/shaders/particle.frag");
			//process
			for (GLuint i = 0; i < nr_particles; ++i) {
				particles.push_back(Particle());
			}
			GLfloat particle_quad[] = {
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
			};
			glGenVertexArrays(1, &this->particleVAO);
			glGenBuffers(1, &particleVBO);
			glBindVertexArray(this->particleVAO);
			// Fill mesh buffer
			glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}
		if (!this->loadmodel_shader) {
			this->loadmodel_shader = new
				Shader(
					"../WaterSurface-master/src/shaders/load_model.vert",
					nullptr, nullptr, nullptr,
					"../WaterSurface-master/src/shaders/load_model.frag");

			snow_texture = new Texture2D("../tile/white.jpg");
			for (int i = 0; i < 7; i++) {
				string temp = "../tile/"+to_string(i)+".jpg";
				firework_color.push_back(Texture2D(temp.c_str()));
			}
		}
		if (!this->building_shader) {
			this->building_shader = new
				Shader(
					"../WaterSurface-master/src/shaders/Instance.vert",
					nullptr, nullptr, nullptr,
					"../WaterSurface-master/src/shaders/load_model.frag");
		}
		if (!this->wave_shader) {
			this->wave_shader = new
				Shader(
					"../WaterSurface-master/src/shaders/wave.vert",
					nullptr, nullptr, nullptr,
					"../WaterSurface-master/src/shaders/wave.frag");
			//reflect
			glGenTextures(1, &reflect_texture);
			glBindTexture(GL_TEXTURE_2D, reflect_texture);

			glGenFramebuffers(1, &reflectFBO);
			glGenRenderbuffers(1, &reflectRBO);

			glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);


			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->pixel_w(), this->pixel_h(), 0.1, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflect_texture, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, reflectRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->pixel_w(), this->pixel_h());
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, reflectRBO);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			//refract
			glGenTextures(1, &refract_texture);
			glBindTexture(GL_TEXTURE_2D, refract_texture);

			glGenFramebuffers(1, &refractFBO);
			glGenRenderbuffers(1, &refractRBO);

			glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);


			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->pixel_w(), this->pixel_h(), 0.1, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refract_texture, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, refractRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->pixel_w(), this->pixel_h());
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, refractRBO);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		if (!this->skybox) {
			this->skybox = new
				Shader(
					"../WaterSurface-master/src/shaders/skybox.vert",
					nullptr, nullptr, nullptr,
					"../WaterSurface-master/src/shaders/skybox.frag");

			float skyboxVertices[] = {
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
			};
			glGenVertexArrays(1, &skyboxVAO);
			glBindVertexArray(skyboxVAO);
			glGenBuffers(1, &skyboxVBO);
			glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glBindVertexArray(0);
			vector<std::string> faces
			{
				"../skybox/right.jpg",
				"../skybox/left.jpg",
				"../skybox/top.jpg",
				"../skybox/bottom.jpg",
				"../skybox/front.jpg",
				"../skybox/back.jpg"

			};
			cubemapTexture = loadCubemap(faces);
		}
		if (!this->screenShader) {
			this->screenShader = new
				Shader(
					"../WaterSurface-master/src/shaders/screenShader.vert",
					nullptr, nullptr, nullptr,
					"../WaterSurface-master/src/shaders/screenShader.frag");

			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			  // positions   // texCoords
			  -1.0f,  1.0f,  0.0f, 1.0f,
			  -1.0f, -1.0f,  0.0f, 0.0f,
			   1.0f, -1.0f,  1.0f, 0.0f,

			  -1.0f,  1.0f,  0.0f, 1.0f,
			   1.0f, -1.0f,  1.0f, 0.0f,
			   1.0f,  1.0f,  1.0f, 1.0f
			};
			// screen quad VAO
			glGenVertexArrays(1, &quadVAO);
			glBindVertexArray(quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			glBindVertexArray(0);
			// framebuffer configuration
			// -------------------------
			glGenFramebuffers(1, &framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			// create a color attachment texture
			glGenTextures(1, &textureColorbuffer);
			glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->pixel_w(), this->pixel_h(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			unsigned int rbo;
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->pixel_w(), this->pixel_h()); // use a single renderbuffer object for both a depth AND stencil buffer.
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		if (!this->interactive_frame) {
			this->interactive_frame = new
				Shader(
					"../WaterSurface-master/src/shaders/interact_frame.vert",
					nullptr, nullptr, nullptr,
					"../WaterSurface-master/src/shaders/interact_frame.frag");

			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			  // positions   // texCoords
			  -1.0f,  1.0f,  0.0f, 1.0f,
			  -1.0f, -1.0f,  0.0f, 0.0f,
			   1.0f, -1.0f,  1.0f, 0.0f,

			  -1.0f,  1.0f,  0.0f, 1.0f,
			   1.0f, -1.0f,  1.0f, 0.0f,
			   1.0f,  1.0f,  1.0f, 1.0f
			};
			// screen quad VAO
			glGenVertexArrays(1, &quadVAO1);
			glBindVertexArray(quadVAO1);
			glGenBuffers(1, &quadVBO1);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO1);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			glBindVertexArray(0);
			// framebuffer configuration
			// -------------------------
			glGenFramebuffers(1, &framebuffer1);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
			// create a color attachment texture
			glGenTextures(1, &textureColorbuffer1);
			glBindTexture(GL_TEXTURE_2D, textureColorbuffer1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->pixel_w(), this->pixel_h(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer1, 0);
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		if (!this->commom_matrices) {
			this->commom_matrices = new UBO();
			this->commom_matrices->size = 2 * sizeof(glm::mat4);
			glGenBuffers(1, &this->commom_matrices->ubo);
			glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
			glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		if (!wave_model) {
			wave_model = new Model("../wave/wave.obj");
		}

		if (!tile_model) {
			tile_model = new Model("../tile/tile.obj");
			tile_texture = new Texture2D("../tile/tiles.jpg");
			tile_texture_LOD = new Texture2D("../tile/tiles.jpg",1);
		}
#ifdef DEBUG
		if (!tunnel_model) {
			tunnel_model = new Model("../tunnel/LeakeStreetTunnel03.obj");
		}
		if (!terrain_model) {
			terrain_model = new Model("../terrain/Landscape01.obj");
		}
		#endif // DEBUG
		if (!person_model) {
			person_model = new Model("../person/gura.obj");
		}
		if (!building_model) {
			building_model = new Model("../building/building.obj");
			srand(clock()); // 初始化随机种子    
			float radius = 50.0;
			float offset = 250.0f;
			for (unsigned int i = 0; i < building_amount; i++)
			{
				glm::mat4 model;
				float angle = (float)i / (float)building_amount * 360.0f;
				float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
				float x = sin(angle) * radius + displacement;
				displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
				//float y = displacement * 0.4f+100.0f; // 让行星带的高度比x和z的宽度要小
				displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
				float z = cos(angle) * radius + displacement;
				model = glm::translate(model, glm::vec3(x+30, 0.0, z+30));

				// 2. 缩放：在 0.05 和 0.25f 之间缩放
				float scale = (rand() % 20) / 10.0f + 0.5;
				model = glm::scale(model, glm::vec3(scale));
				building_pos.push_back(glm::vec2(x, z));
				// 4. 添加到矩阵的数组中
				building_modelMatrices.push_back(model);
			}
		}
		if (!snow_model) {
			snow_model = new Model("../snow/snow.obj");
			snow_amount *= 5;
			for (unsigned int i = 0; i < snow_amount; i++)
			{
				float x = rand()%2000-1000;
				float z = rand() % 2000 - 1000;
				float y = rand() % 1000+200;
				snow_pos.push_back(glm::vec3(x,y,z));
			}
			modelMatrices = new glm::mat4[snow_amount];
			glGenBuffers(1, &snow_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, snow_buffer);
			glBufferData(GL_ARRAY_BUFFER, snow_amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
			for (unsigned int i = 0; i < snow_model->meshes.size(); i++)
			{
				unsigned int VAO = snow_model->meshes[i].VAO;
				glBindVertexArray(VAO);
				// set attribute pointers for matrix (4 times vec4)
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
				glEnableVertexAttribArray(5);
				glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
				glEnableVertexAttribArray(6);
				glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

				glVertexAttribDivisor(3, 1);
				glVertexAttribDivisor(4, 1);
				glVertexAttribDivisor(5, 1);
				glVertexAttribDivisor(6, 1);

				glBindVertexArray(0);
			}
		}
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	// Set up the view port
	glViewport(0, 0, w(), h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[] = { 0,1,1,0 };
	GLfloat lightPosition2[] = { 1, 0, 0, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .1f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	//transformation matrix
	glm::mat4 wave_transfer = glm::mat4(1.0f);
	//wave_transfer = glm::translate(wave_transfer, glm::vec3(0.0f, 100, 0));
	wave_transfer = glm::scale(wave_transfer, glm::vec3(scale, scale, scale));

	//get Projection and view
	glGetFloatv(GL_PROJECTION_MATRIX, Projection);
	glGetFloatv(GL_MODELVIEW_MATRIX, View);
	glm::mat4 View_mat4x4 = glm::make_mat4x4(View);
	glm::mat4 Projection_mat4x4 = glm::make_mat4x4(Projection);
	glm::mat4 skybox_View = glm::mat4(glm::mat3(View_mat4x4));
	glm::mat4 viewMatrix = glm::inverse(View_mat4x4);
	glm::vec3 viewPos(viewMatrix[3][0], viewMatrix[3][1], viewMatrix[3][2]);
	camPos = viewPos;
	//reflect
	if (true) {
		glm::vec3 p0 = glm::vec3(-1, 0, 1);
		glm::vec3 n = glm::vec3(0, 1, 0);

		float mA = n.x;
		float mB = n.y;
		float mC = n.z;
		float mD = -n.x * p0.x - n.y * p0.y - n.z * p0.z;;
		glm::mat4 reflectMat;
		reflectMat[0][0] = -2 * n.x * n.x + 1;
		reflectMat[0][1] = -2 * n.x * n.y;
		reflectMat[0][2] = -2 * n.x * n.z;
		reflectMat[0][3] = -2 * n.x * mD;

		reflectMat[1][0] = -2 * n.x * n.y;
		reflectMat[1][1] = -2 * n.y * n.y + 1;
		reflectMat[1][2] = -2 * n.y * n.z;
		reflectMat[1][3] = -2 * n.y * mD;

		reflectMat[2][0] = -2 * n.z * n.x;
		reflectMat[2][1] = -2 * n.z * n.y;
		reflectMat[2][2] = -2 * n.z * n.z + 1;
		reflectMat[2][3] = -2 * n.z * mD;

		reflectMat[3][0] = 0;
		reflectMat[3][1] = 0;
		reflectMat[3][2] = 0;
		reflectMat[3][3] = 1;

		glm::mat4 viewPrime = View_mat4x4 * reflectMat;
		float fov = 2.0 * atan(1.0 / Projection_mat4x4[1][1]) * 180.0 / 3.1415926;
		glm::mat4 _projection_matrix = glm::perspective<float>(glm::radians(fov), 1.0f, 0.01, 1000.0f);
		glm::vec4 newClipPlane = glm::transpose(glm::inverse(viewPrime)) * glm::vec4(n, mD);
		glm::vec4 q = glm::vec4((glm::sign(newClipPlane.x) + _projection_matrix[2][0]) / _projection_matrix[0][0],
			(glm::sign(newClipPlane.y) + _projection_matrix[2][1]) / _projection_matrix[1][1],
			-1.0f, (1.0f + _projection_matrix[2][2]) / _projection_matrix[3][2]);
		glm::vec4 c = newClipPlane * (2.0f / glm::dot(newClipPlane, q));
		_projection_matrix[0][2] = c.x;
		_projection_matrix[1][2] = c.y;
		_projection_matrix[2][2] = c.z + 1.0f;
		_projection_matrix[3][2] = c.w;
		glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		/*Sky box*/
		glDepthFunc(GL_LEQUAL);
		skybox->Use();
		glm::mat4 skyboxView = glm::mat4(glm::mat3(viewPrime));
		glUniformMatrix4fv(glGetUniformLocation(skybox->Program, "proj_matrix"), 1, GL_FALSE, &_projection_matrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(skybox->Program, "model_matrix"), 1, GL_FALSE, &skyboxView[0][0]);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		//draw tiles
		loadmodel_shader->Use();
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "proj_matrix"), 1, GL_FALSE, &_projection_matrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "view_matrix"), 1, GL_FALSE, &viewPrime[0][0]);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);
		if (tw->lod)
			tile_texture_LOD->bind(11);
		else
			tile_texture->bind(11);
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &wave_transfer[0][0]);
		glUniform1i(glGetUniformLocation(loadmodel_shader->Program, "texture_diffuse1"), 11);
		tile_model->Draw(*loadmodel_shader);
		if (tw->lod)
			tile_texture_LOD->unbind(11);
		else
			tile_texture->unbind(11);
		glDisable(GL_CULL_FACE);

		//draw building
		loadmodel_shader->Use();
		for (unsigned int i = 0; i < building_amount; i++)
		{
			float building_angle = atan2(building_pos[i].x - viewPos.x, building_pos[i].y - viewPos.z) * (180 / 3.1415926535);
			glm::mat4 building_transfer = building_modelMatrices[i];
			building_transfer = glm::rotate(building_transfer, glm::radians(building_angle + 90), glm::vec3(0.0, 1.0, 0.0));
			glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &building_transfer[0][0]);
			building_model->Draw(*loadmodel_shader);
		}

		//draw gura
		loadmodel_shader->Use();
		glm::mat4 person_transfer = glm::mat4(1.0f);
		person_transfer = glm::translate(person_transfer, personPos);
		person_transfer = glm::scale(person_transfer, glm::vec3(0.5, 0.5, 0.5));
		person_transfer = person_transfer * train_rotate;
		person_transfer = glm::rotate(person_transfer, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &person_transfer[0][0]);
		person_model->Draw(*loadmodel_shader);

		if (firework_over) {
			firework_over = false;
			firework_init();
			firework_clock = clock();
		}
		else {
			if (clock() - firework_clock > 30) {
				firework_clock = clock();
				firework_update();
			}
		}

		for (unsigned int i = 0; i < fire_pos.size(); i++)
		{
			firework_color[((int)(floor(fire_angle[i] / 52)) + firework_radius % 7) % 7].bind(14);
			glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &fire_pos[i][0][0]);
			glUniform1i(glGetUniformLocation(loadmodel_shader->Program, "texture_diffuse1"), 14);
			snow_model->Draw(*loadmodel_shader);
			firework_color[i % 7].unbind(14);
		}

		/*snow_update();
		snow_texture->bind(15);
		for (unsigned int i = 0; i < snow_amount; i++)
		{
			glm::mat4 snow_transfer(1);
			snow_transfer = glm::translate(snow_transfer, snow_pos[i]);
			glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &snow_transfer[0][0]);
			glUniform1i(glGetUniformLocation(loadmodel_shader->Program, "texture_diffuse1"), 15);
			snow_model->Draw(*loadmodel_shader);
		}
		snow_texture->unbind(15);*/
		//Draw snow
		snow_update();
		snow_texture->bind(15);
		glBindBuffer(GL_ARRAY_BUFFER, snow_buffer);
		glBufferData(GL_ARRAY_BUFFER, snow_amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
		building_shader->Use();
		glUniformMatrix4fv(glGetUniformLocation(building_shader->Program, "view_matrix"), 1, GL_FALSE, View);
		glUniformMatrix4fv(glGetUniformLocation(building_shader->Program, "proj_matrix"), 1, GL_FALSE, Projection);
		glUniform1i(glGetUniformLocation(building_shader->Program, "texture_diffuse1"), 15);
		for (unsigned int i = 0; i < snow_model->meshes.size(); i++)
		{
			glBindVertexArray(snow_model->meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, snow_model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, snow_amount);
			glBindVertexArray(0);
		}
		snow_texture->unbind(15);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	if (true)
	{
		glm::vec3 p0 = glm::vec3(-1, 0, 1);
		glm::vec3 n = glm::vec3(0, 1, 0);

		float mA = n.x;
		float mB = n.y;
		float mC = n.z;
		float mD = -n.x * p0.x - n.y * p0.y - n.z * p0.z;;

		glm::mat4 viewPrime = View_mat4x4 * glm::scale(glm::vec3(1, glm::clamp(1.0f - (0.3f), 0.001f, 1.0f), 1));
		float fov = 2.0 * atan(1.0 / Projection_mat4x4[1][1]) * 180.0 / 3.1415926;
		glm::mat4 _projection_matrix = glm::perspective<float>(glm::radians(fov), 1.0f, 0.01, 1000.0f);
		glm::vec4 newClipPlane = glm::transpose(glm::inverse(viewPrime)) * glm::vec4(n, mD);
		(&_projection_matrix[0][0])[2] = newClipPlane.x + (&_projection_matrix[0][0])[3];//x
		(&_projection_matrix[0][0])[6] = newClipPlane.y + (&_projection_matrix[0][0])[7];//y
		(&_projection_matrix[0][0])[10] = newClipPlane.z + (&_projection_matrix[0][0])[11];//z
		(&_projection_matrix[0][0])[14] = newClipPlane.w + (&_projection_matrix[0][0])[15];//w

		glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//draw tiles
		loadmodel_shader->Use();
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "proj_matrix"), 1, GL_FALSE, &_projection_matrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "view_matrix"), 1, GL_FALSE, &viewPrime[0][0]);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		if (tw->lod)
			tile_texture_LOD->bind(11);
		else
			tile_texture->bind(11);
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &wave_transfer[0][0]);
		glUniform1i(glGetUniformLocation(loadmodel_shader->Program, "texture_diffuse1"), 11);
		tile_model->Draw(*loadmodel_shader);
		if (tw->lod)
			tile_texture_LOD->unbind(11);
		else
			tile_texture->unbind(11);
		glDisable(GL_CULL_FACE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	if (tw->direct || tw->spot || tw->point) {
		//Frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
	

	if (tw->waveBrowser->value() == 3) {
		if (load_heightmap) {
			for (int i = 0; i < 200; i++) {
				string name = to_string(i);
				if (name.size() == 1) {
					name = "00" + name;
				}
				else if (name.size() == 2) {
					name = "0" + name;
				}
				name = "../height_map/" + name + ".png";
				height_id.push_back(Texture2D(name.c_str()));
			}
			load_heightmap = false;
		}
		height_index = height_index+tw->WaveFrequency->value();
		if (height_index > 200) {
			height_index = 0;
		}
		height_id[(int)(height_index + tw->WaveFrequency->value()) % 200].bind(5);
	}

	

	



#ifdef particlee
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	currentime = clock();
	dt = (currentime - lastime) / 400;
	lastime = currentime;

	GLuint nr_new_particles = nr_particles /60;
	// Add new particles
	for (GLuint i = 0; i < nr_new_particles; ++i)
	{
		int unusedParticle = FirstUnusedParticle();
		RespawnParticle(particles[unusedParticle]);
	}
	// Update all particles
	for (GLuint i = 0; i < nr_particles; ++i)
	{
		Particle& p = particles[i];
		p.Life -= dt; // reduce life
		if (p.Life > 0.0f)
		{   // particle is alive, thus update
			p.Color.a = 1-p.Life;
			p.Scale = pow(1.0-p.Life,1);
			p.Module = glm::mat4(1.0f);
			p.Module = glm::translate(p.Module, glm::vec3(p.trainPos.x + p.X_random * 0.5* p.Scale, p.trainPos.y + 10 + p.Y_random * 0.5* p.Scale, p.trainPos.z + p.Z_random * 0.5* p.Scale));
			p.Module = glm::scale(p.Module, glm::vec3(p.Scale_randon, p.Scale_randon, p.Scale_randon));
		}
	}
	particle_shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(particle_shader->Program, "view_matrix"), 1, GL_FALSE, View);
	glUniformMatrix4fv(glGetUniformLocation(particle_shader->Program, "proj_matrix"), 1, GL_FALSE, Projection);
	Draw();
#endif





	wave_shader->Use();
	glUniform3f(glGetUniformLocation(wave_shader->Program, "viewPos"), viewPos.x, viewPos.y, viewPos.z);
	/*Wave*/
	glUniform1i(glGetUniformLocation(wave_shader->Program, "height_map_texture"), 5);
	glUniform1f(glGetUniformLocation(wave_shader->Program, "amplitude"), tw->WaveAmplitude->value());
	glUniform1f(glGetUniformLocation(wave_shader->Program, "frequency"), tw->WaveFrequency->value());
	glUniform1f(glGetUniformLocation(wave_shader->Program, "t"), tw->wave_t);
	glUniform1f(glGetUniformLocation(wave_shader->Program, "reflect_enable"), tw->reflect);
	glUniform1f(glGetUniformLocation(wave_shader->Program, "refract_enable"), tw->refract);
	glUniform1i(glGetUniformLocation(wave_shader->Program, "wave_mode"), tw->waveBrowser->value());

	glUniformMatrix4fv(glGetUniformLocation(wave_shader->Program, "proj_matrix"), 1, GL_FALSE, Projection);
	glUniformMatrix4fv(glGetUniformLocation(wave_shader->Program, "view_matrix"), 1, GL_FALSE, View);
	glUniformMatrix4fv(glGetUniformLocation(wave_shader->Program, "model_matrix"), 1, GL_FALSE, &wave_transfer[0][0]);

	glUniform2f(glGetUniformLocation(wave_shader->Program, "uv_center"), uv_center.x, uv_center.y);
	glUniform1f(glGetUniformLocation(wave_shader->Program, "uv_t"), uv_t);

	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, reflect_texture);
	glUniform1i(glGetUniformLocation(wave_shader->Program, "reflect_texture"), 13);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, refract_texture);
	glUniform1i(glGetUniformLocation(wave_shader->Program, "refract_texture"), 14);

	wave_model->Draw(*wave_shader);
	if (tw->waveBrowser->value() == 3) {
		height_id[height_index].unbind(5);
	}
	/*Sky box*/
	glDepthFunc(GL_LEQUAL);
	skybox->Use();
	glUniformMatrix4fv(glGetUniformLocation(skybox->Program, "proj_matrix"), 1, GL_FALSE, Projection);
	glUniformMatrix4fv(glGetUniformLocation(skybox->Program, "model_matrix"), 1, GL_FALSE, &skybox_View[0][0]);
	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);





	loadmodel_shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "proj_matrix"), 1, GL_FALSE, Projection);
	glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "view_matrix"), 1, GL_FALSE, View);
#ifdef DEBUG
	//Draw terrain
	glm::mat4 terrain_transfer = glm::mat4(1.0f);
	terrain_transfer = glm::translate(terrain_transfer, glm::vec3(0.0f, -110.0f,-17.39));
	terrain_transfer = glm::scale(terrain_transfer, glm::vec3(220.0, 220.0, 220.0));
	terrain_transfer = glm::rotate(terrain_transfer, glm::radians(27.39f), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &terrain_transfer[0][0]);
	terrain_model->Draw(*loadmodel_shader);

	//Draw Tunnel
	glm::mat4 tunnel_transfer = glm::mat4(1.0f);
	tunnel_transfer = glm::translate(tunnel_transfer, glm::vec3(70.0f, 10.0f, 56.52));
	tunnel_transfer = glm::scale(tunnel_transfer, glm::vec3(6.0, 6.0, 6.0));
	tunnel_transfer = glm::rotate(tunnel_transfer, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &tunnel_transfer[0][0]);
	tunnel_model->Draw(*loadmodel_shader);
#endif // DEBUG
	//Draw Person
	glm::mat4 person_transfer = glm::mat4(1.0f);
	person_transfer = glm::translate(person_transfer, personPos);
	person_transfer = glm::scale(person_transfer, glm::vec3(0.5, 0.5, 0.5));
	person_transfer = person_transfer * train_rotate;
	person_transfer = glm::rotate(person_transfer, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &person_transfer[0][0]);
	person_model->Draw(*loadmodel_shader);

	//Draw Tiles
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	if (tw->lod)
	tile_texture_LOD->bind(11);
	else
	tile_texture->bind(11);
	glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &wave_transfer[0][0]);
	glUniform1i(glGetUniformLocation(loadmodel_shader->Program, "texture_diffuse1"), 11);
	tile_model->Draw(*loadmodel_shader);
	if (tw->lod)
		tile_texture_LOD->unbind(11);
	else
		tile_texture->unbind(11);
	glDisable(GL_CULL_FACE);

	//Draw firework
	if (firework_over) {
		firework_over = false;
		firework_init();
		firework_clock = clock();
	}
	else {
		if (clock() - firework_clock > 100) {
			firework_clock = clock();
			firework_update();
		}
	}
	for (unsigned int i = 0; i < fire_pos.size(); i++)
	{
		//firework_color[((int)(floor(fire_angle[i] / 52)))].bind(14);
		firework_color[((int)(floor(fire_angle[i]/52))+firework_radius%7)%7].bind(14);
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &fire_pos[i][0][0]);
		glUniform1i(glGetUniformLocation(loadmodel_shader->Program, "texture_diffuse1"), 14);
		snow_model->Draw(*loadmodel_shader);
		//firework_color[((int)(floor(fire_angle[i] / 52)))].bind(14);
		firework_color[((int)(floor(fire_angle[i] / 52)) + firework_radius % 7) % 7].unbind(14);
	}

	for (unsigned int i = 0; i < building_amount; i++)
	{
		float building_angle = atan2(building_pos[i].x - viewPos.x, building_pos[i].y - viewPos.z) * (180 / 3.1415926535);
		glm::mat4 building_transfer = building_modelMatrices[i];
		building_transfer = glm::rotate(building_transfer, glm::radians(building_angle + 90), glm::vec3(0.0, 1.0, 0.0));
		glUniformMatrix4fv(glGetUniformLocation(loadmodel_shader->Program, "model_matrix"), 1, GL_FALSE, &building_transfer[0][0]);
		building_model->Draw(*loadmodel_shader);
	}
	//Draw snow
	snow_update();
	snow_texture->bind(15);
	glBindBuffer(GL_ARRAY_BUFFER, snow_buffer);
	glBufferData(GL_ARRAY_BUFFER, snow_amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
	building_shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(building_shader->Program, "view_matrix"), 1, GL_FALSE, View);
	glUniformMatrix4fv(glGetUniformLocation(building_shader->Program, "proj_matrix"), 1, GL_FALSE, Projection);
	glUniform1i(glGetUniformLocation(building_shader->Program, "texture_diffuse1"), 15);
	for (unsigned int i = 0; i < snow_model->meshes.size(); i++)
	{
		glBindVertexArray(snow_model->meshes[i].VAO);
		glDrawElementsInstanced(GL_TRIANGLES, snow_model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, snow_amount);
		glBindVertexArray(0);
	}
	snow_texture->unbind(15);
	

	glUseProgram(0);
	setupObjects();
	drawStuff();
	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}


	if (tw->direct || tw->spot || tw->point) {
		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
		glClear(GL_COLOR_BUFFER_BIT);
		this->screenShader->Use();
		glUniform1f(glGetUniformLocation(screenShader->Program, "pixel_enable"), tw->direct);
		glUniform1f(glGetUniformLocation(screenShader->Program, "offset_enable"), tw->point);
		glUniform1f(glGetUniformLocation(screenShader->Program, "other_enable"), tw->spot);
		glUniform1f(glGetUniformLocation(screenShader->Program, "rt_w"), w());
		glUniform1f(glGetUniformLocation(screenShader->Program, "rt_h"), h());
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
		glUniform1i(glGetUniformLocation(screenShader->Program, "screenTexture"), 12);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}

//************************************************************************
//
// * This sets up both the Projection and the View matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		} 
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);
	} 
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
		trainCamView(this,aspect);
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::trainCamView(TrainView* TrainV, float aspect) {
	float percent = 1.0f / DIVIDE_LINE;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120, aspect, 0.01, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float t;
	int i;
	if (TrainV->tw->arcLength->value()) {
		t = t_t;
		i = t_i;
	}
	else {
		i = floor(t_time);
		t = t_time - i;
	}
	ControlPoint p1 = m_pTrack->points[(i - 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
	ControlPoint p2 = m_pTrack->points[(i + m_pTrack->points.size()) % m_pTrack->points.size()];
	ControlPoint p3 = m_pTrack->points[(i + 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
	ControlPoint p4 = m_pTrack->points[(i + 2 + m_pTrack->points.size()) % m_pTrack->points.size()];

	Pnt3f eye_orient = GMT(p1.orient, p2.orient, p3.orient, p4.orient, tw->splineBrowser->value(), t - percent);
	Pnt3f centery_ori = GMT(p1.orient, p2.orient, p3.orient, p4.orient, tw->splineBrowser->value(), t);

	


	Pnt3f eye = GMT(p1.pos, p2.pos, p3.pos, p4.pos, tw->splineBrowser->value(), t - percent) + eye_orient * 2.0f;
	Pnt3f centery = GMT(p1.pos, p2.pos, p3.pos, p4.pos, tw->splineBrowser->value(), t) + centery_ori * 2.0f;
	gluLookAt(eye.x, eye.y, eye.z, centery.x, centery.y, centery.z, eye_orient.x, eye_orient.y, eye_orient.z);

	trainPos = glm::vec3(centery.x, centery.y, centery.z);
}
void TrainView::drawTrack(TrainView* TrainV, bool doingShadows) {
	Path_Total = 0;
	float Sleep_Total = 0.0f;
	float percent = 1.0f / DIVIDE_LINE;
	Pnt3f lastqt;
	bool Draw_Sleeper = false;
	bool check = false;
	int count = 0;
	for (size_t i = 0; i < m_pTrack->points.size(); i++) {
		float t = 0;
		ControlPoint p1 = m_pTrack->points[(i - 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
		ControlPoint p2 = m_pTrack->points[(i + m_pTrack->points.size()) % m_pTrack->points.size()];
		ControlPoint p3 = m_pTrack->points[(i + 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
		ControlPoint p4 = m_pTrack->points[(i + 2 + m_pTrack->points.size()) % m_pTrack->points.size()];
		for (size_t j = 0; j < DIVIDE_LINE; j++) {
			Pnt3f qt0 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), t);
			Pnt3f orient_t = GMT(p1.orient, p2.orient, p3.orient, p4.orient, TrainV->tw->splineBrowser->value(), t);
			Pnt3f qt1 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), t += percent);
			Pnt3f forward = qt1 - qt0;
			Pnt3f cross_t = forward * orient_t;
			cross_t.normalize();
			orient_t = cross_t * forward;
			orient_t.normalize();
			cross_t = cross_t * Sleeper_Width;
			/*火車移動*/
			Path_Total += sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
			if (!check && Path_Total > TrainV->m_pTrack->trainU) {
				if (qt1.y - qt0.y > 0) {
					physical = qt0.y - qt1.y;
				}
				else if (qt1.y - qt0.y < 0) {
					physical = qt0.y - qt1.y;
					physical *= 0.7;
				}
				else {
					physical = 0;
				}
				t_t = t;
				t_i = i;
				check = true;
			}

			/*鐵軌*/
			if (!doingShadows) {
				glColor3ub(77, 19, 0);
			}
			glNormal3f(orient_t.x, orient_t.y, orient_t.z);
			glLineWidth(5);
			glBegin(GL_LINES);
			glVertex3f_Simplify(qt0 + cross_t);
			glVertex3f_Simplify(qt1 + cross_t);
			glVertex3f_Simplify(qt0 - cross_t);
			glVertex3f_Simplify(qt1 - cross_t);
			glEnd();
			//補畫鐵軌斷裂處
			if (j != 0) {
				glNormal3f(orient_t.x, orient_t.y, orient_t.z);
				glLineWidth(5);
				glBegin(GL_LINES);
				glVertex3f_Simplify(lastqt + cross_t);
				glVertex3f_Simplify(qt1 + cross_t);
				glVertex3f_Simplify(lastqt - cross_t);
				glVertex3f_Simplify(qt1 - cross_t);
				glEnd();
			}
			Sleep_Total += sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
			if (!Draw_Sleeper && Sleep_Total >= Sleeper_Length) {
				count++;
				forward.normalize();
				DrawSleeper(qt0, qt0 - forward * Sleeper_Length, cross_t, orient_t, doingShadows);
				/*if (count % 3 == 0) {
					DrawPillar(qt0, qt0 - forward * Sleeper_Length, cross_t, orient_t, doingShadows);
					count = 0;
				}*/
				Sleep_Total -= Sleeper_Length;
				Draw_Sleeper = !Draw_Sleeper;
			}
			else if (Draw_Sleeper && Sleep_Total >= Sleeper_Interval) {
				Sleep_Total -= Sleeper_Interval;
				Draw_Sleeper = !Draw_Sleeper;
			}
			lastqt = qt0;
		}
	}
}

void TrainView::drawTrain(TrainView* TrainV, bool doingShadows) {
	float percent = 1.0f / DIVIDE_LINE;
	float t;
	int i;
	if (TrainV->tw->arcLength->value()) {
		t = t_t;
		i = t_i;
	}
	else {
		i = floor(t_time);
		t = t_time - i;
	}

	for (int j = 1; j <= car_quantity; j++) {
		if (TrainV->tw->arcLength->value()) {
			bool check = false;
			ControlPoint p1 = m_pTrack->points[(i - 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
			ControlPoint p2 = m_pTrack->points[(i + m_pTrack->points.size()) % m_pTrack->points.size()];
			ControlPoint p3 = m_pTrack->points[(i + 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
			ControlPoint p4 = m_pTrack->points[(i + 2 + m_pTrack->points.size()) % m_pTrack->points.size()];

			Pnt3f qt0 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), t);
			Pnt3f orient_t = GMT(p1.orient, p2.orient, p3.orient, p4.orient, TrainV->tw->splineBrowser->value(), t);
			Pnt3f qt1 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), t += percent);

			Pnt3f cross_t = (qt1 - qt0) * orient_t;
			cross_t.normalize();
			orient_t = cross_t * (qt1 - qt0);
			orient_t.normalize();
			Pnt3f forward = (qt1 - qt0);
			forward.normalize();
			train_rotate = {
				forward.x,forward.y,forward.z,0.0f,
				orient_t.x,orient_t.y,orient_t.z,0.0f,
				cross_t.x,cross_t.y,cross_t.z,0.0f,
				0.0f,0.0f,0.0f,1.0f,
			};
			cross_t = cross_t * Train_Width;
			Pnt3f up = orient_t * Train_Height;
			forward = forward * Train_Forward;


			
			trainPos = glm::vec3(qt1.x, qt1.y, qt1.z) + glm::vec3(forward.x, forward.y, forward.z);
			personPos = glm::vec3(qt0.x, qt0.y, qt0.z) + glm::vec3(forward.x*0.8, forward.y*0.8, forward.z*0.8);

			float total = 0.0f;
			if (j == 1) {
				DrawTrainHead(qt0, cross_t, up, forward, doingShadows);
			}
			else {
				DrawTrain(qt0, cross_t, up, forward, doingShadows);
			}

			while (!check) {
				for (t; t >= 0; t = t - percent) {
					qt0 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), t);
					qt1 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), t + percent);
					forward = qt1 - qt0;
					total += sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
					if (total >= 13) {
						check = true;
						break;
					}
				}
				if (check) {
					break;
				}
				t = 1.0f;
				i -= 1;
				if (i < 0) {
					i = m_pTrack->points.size() - 1;
				}
				p1 = m_pTrack->points[(i - 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
				p2 = m_pTrack->points[(i + m_pTrack->points.size()) % m_pTrack->points.size()];
				p3 = m_pTrack->points[(i + 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
				p4 = m_pTrack->points[(i + 2 + m_pTrack->points.size()) % m_pTrack->points.size()];
			}
		}
		else {
			float tt = t_time - percent * 200 * (j - 1);
			int ii = floor(tt);
			tt -= ii;
			ControlPoint p1 = m_pTrack->points[(ii - 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
			ControlPoint p2 = m_pTrack->points[(ii + m_pTrack->points.size()) % m_pTrack->points.size()];
			ControlPoint p3 = m_pTrack->points[(ii + 1 + m_pTrack->points.size()) % m_pTrack->points.size()];
			ControlPoint p4 = m_pTrack->points[(ii + 2 + m_pTrack->points.size()) % m_pTrack->points.size()];
			
			Pnt3f qt0 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), tt);
			Pnt3f orient_t = GMT(p1.orient, p2.orient, p3.orient, p4.orient, TrainV->tw->splineBrowser->value(), tt);
			Pnt3f qt1 = GMT(p1.pos, p2.pos, p3.pos, p4.pos, TrainV->tw->splineBrowser->value(), tt += percent);

			Pnt3f cross_t = (qt1 - qt0) * orient_t;
			cross_t.normalize();
			orient_t = cross_t * (qt1 - qt0);
			orient_t.normalize();
			cross_t = cross_t * Train_Width;
			Pnt3f up = orient_t * Train_Height;
			Pnt3f forward = (qt1 - qt0);
			forward.normalize();
			forward = forward * Train_Forward;
			if (j == 1) {
				DrawTrainHead(qt0, cross_t, up, forward, doingShadows);
			}
			else {		
				DrawTrain(qt0, cross_t, up, forward, doingShadows);
			}
		}
	}
}
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for(size_t i=0; i<m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if ( ((int) i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################
	drawTrack(this, doingShadows);
	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	//since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();		

	// where is the mouse?
	int mx = Fl::event_x(); 
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
						5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for(size_t i=0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint) (i+1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3]-1;
	} else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n",selectedCube);


	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
	glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
	// make sure we clear the framebuffer's content
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	interactive_frame->Use();


	//transformation matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(glm::translate(model, glm::vec3(0, 10, 0)), glm::vec3(20, 20, 20));
	glUniformMatrix4fv(glGetUniformLocation(interactive_frame->Program, "proj_matrix"), 1, GL_FALSE, Projection);
	glUniformMatrix4fv(glGetUniformLocation(interactive_frame->Program, "view_matrix"), 1, GL_FALSE, View);
	glUniformMatrix4fv(glGetUniformLocation(interactive_frame->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	wave_model->Draw(*interactive_frame);

	//pick
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glm::vec3 uv;
	glReadPixels(Fl::event_x(),h()- Fl::event_y()-1, 1, 1, GL_RGB, GL_FLOAT, &uv[0]);

	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	if (uv.b != 1.0) {
		uv_center.x = uv.x;
		uv_center.y = uv.y;
		uv_t = tw->wave_t;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void TrainView::setUBO()
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	//HMatrix view_matrix; 
	//this->arcball.getMatrix(view_matrix);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	//projection_matrix = glm::perspective(glm::radians(this->arcball.getFoV()), (GLfloat)wdt / (GLfloat)hgt, 0.01f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
