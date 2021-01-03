#pragma once

#include <glad/glad.h>
#include <stdlib.h>
#include <ctime>
#include <cmath>


struct cube_particle
{
	GLfloat xpos;//(xpos,ypos,zpos)��particle��position
	GLfloat ypos;
	GLfloat zpos;
	GLfloat xspeed;//(xspeed,yspeed,zspeed)��particle��speed 
	GLfloat yspeed;
	GLfloat zspeed;
	GLfloat r;//(r,g,b)��particle��color
	GLfloat g;
	GLfloat b;
	GLfloat life;// particle���ةR 
	GLfloat fade;// particle���I��t��
	GLfloat size;// particle���j�p  
	GLbyte bFire;
	GLbyte nExpl;//����particle�ĪG  
	GLbyte bAddParts;// particle�O�_�t������
	GLfloat AddSpeed;//���ڲɤl���[�t��  
	GLfloat AddCount;//���ڲɤl���W�[�q  
	cube_particle* pNext;//�U�@particle 
	cube_particle* pPrev;//�W�@particle   
};


void InitParticle(cube_particle& ep);
void AddParticle(cube_particle ex);
void DeleteParticle(cube_particle** p);

void Explosion1(cube_particle* par);
void Explosion2(cube_particle* par);
void Explosion3(cube_particle* par);
void Explosion4(cube_particle* par);
void Explosion5(cube_particle* par);
void Explosion6(cube_particle* par);
void Explosion7(cube_particle* par);

void ProcessParticles();
void DrawParticles();


class Particle2
{
public:
};