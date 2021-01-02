#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GL/glu.h>
#include<iostream>
using namespace std;
class Texture2D
{
public:
	enum Type {
		TEXTURE_DEFAULT = 0,
		TEXTURE_DIFFUSE, TEXTURE_SPECULAR,
		TEXTURE_NORMAL, TEXTURE_DISPLACEMENT,
		TEXTURE_HEIGHT,
	};

	Type type;

	Texture2D(const char* path, Type texture_type = Texture2D::TEXTURE_DEFAULT):
		type(texture_type)
	{
		cv::Mat img;
		//cv::imread(path, cv::IMREAD_COLOR).convertTo(img, CV_32FC3, 1 / 255.0f);	//unsigned char to float
		img = cv::imread(path, cv::IMREAD_COLOR);

		this->size.x = img.cols;
		this->size.y = img.rows;
		
		//cv::cvtColor(img, img, CV_BGR2RGB);

		glGenTextures(1, &this->id);

		glBindTexture(GL_TEXTURE_2D, this->id);
		int w= this->size.x, h= this->size.y;
		uchar* img_data[10];
		for (int i = 0; i < 10; i++) {
			img_data[i] =new uchar[w*h*3];
			gluScaleImage(GL_RGB, this->size.x, this->size.y, GL_UNSIGNED_BYTE, img.data, w, h, GL_UNSIGNED_BYTE, img_data[i]);
			glTexImage2D(GL_TEXTURE_2D, i, GL_RGB8, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, img_data[i]);
			w /= 2;
			h /= 2;
		}
		//cout << img.data << endl;
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		/*if (img.type() == CV_8UC3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img_data[0]);
		else if (img.type() == CV_8UC4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.cols, img.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);*/
		glBindTexture(GL_TEXTURE_2D, 0);

		img.release();
	}
	void bind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, this->id);
	}
	static void unbind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glm::ivec2 size;
private:
	GLuint id;

};