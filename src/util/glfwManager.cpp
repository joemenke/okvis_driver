#include "glfwManager.h"
#include <iostream>

namespace MyGUI{

std::map<std::string,Window*> Manager::windows;


int (&Manager::init)()= glfwInit;

void (&Manager::terminate)()= glfwTerminate;


bool Manager::running(){
	return windows.size()>0;
}

void Manager::update(){
	for(const auto& win : windows){
		if(!win.second->display()){
			break;
        }else{
            win.second->keyboard_control();
        }
	}
    // Check for any input, or window movement
    glfwPollEvents();

}




Window::Window(std::string name, int resX, int resY):
name_(name){
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	win_ptr= glfwCreateWindow(resX, resY, name.c_str(), NULL, NULL);
	if(win_ptr!=NULL){
		glfwMakeContextCurrent(win_ptr);

		glEnable(GL_DEPTH_TEST); // Depth Testing
    	glDepthFunc(GL_LEQUAL);
    	glDisable(GL_CULL_FACE);
    	glCullFace(GL_BACK);

    	Manager::windows[name] = this;

	}
}

Window::~Window(){

    Manager::windows.erase(name_);  
    glfwDestroyWindow(win_ptr);
}

void Window::add_control_func(GLFWkeyfun controls){
    if(win_ptr!=NULL){
        glfwMakeContextCurrent(win_ptr);
        glfwSetKeyCallback(win_ptr, controls);
    }
}

bool Window::display(){
    if(win_ptr==NULL)
        return false;
    if(!glfwWindowShouldClose(win_ptr)){
        return true;
    }else{
        Manager::windows.erase(name_);
        glfwDestroyWindow(win_ptr);
        win_ptr=NULL;
        return false;
    }

}

bool ImageWindow::display(){
    if(win_ptr==NULL)
        return false;
    if(!glfwWindowShouldClose(win_ptr)){
        if(current_image==NULL)
            return true;
        glfwMakeContextCurrent(win_ptr);
        GLint windowWidth, windowHeight;
        glfwGetFramebufferSize(win_ptr, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetWindowSize(win_ptr, &windowWidth, &windowHeight);
        glLoadIdentity();
        glOrtho(0, windowWidth, windowHeight, 0, -1, +1);
        glBindTexture(GL_TEXTURE_2D, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, (current_image->step & 3) ? 1 : 4);
        int height = current_image->rows;
        int width = current_image->cols;
        int elem_size = current_image->elemSize();
        int stride = current_image->step/elem_size;
        glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, stride, height, 0, image_format_, data_type_, current_image->ptr());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, texture);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0,    0   );
        glTexCoord2f(1, 0); glVertex2f(0+width, 0   );
        glTexCoord2f(1, 1); glVertex2f(0+width, 0+height);
        glTexCoord2f(0, 1); glVertex2f(0,    0+height);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);  
        glfwSwapBuffers(win_ptr);
        return true;
    }else{
        Manager::windows.erase(name_);
        glfwDestroyWindow(win_ptr);
        win_ptr=NULL;
        current_image = NULL;
        return false;
    }

}

void ImageWindow::set_image(std::shared_ptr<cv::Mat> image_in){
    if(win_ptr!=NULL)
        current_image = image_in;
}

ObjectWindow::ObjectWindow(std::string name, int resX, int resY):
Window(name,resX,resY),
eye(Eigen::Vector3d(0,40,40)),
gaze(Eigen::Vector3d(0,0,0)){}


ObjectWindow::~ObjectWindow(){
	for(const auto obj : objects){
		obj.second->windows.erase(name_);
	}
}


bool ObjectWindow::display(){
	if(win_ptr==NULL)
		return false;
	if(!glfwWindowShouldClose(win_ptr)){
		glfwMakeContextCurrent(win_ptr);
		GLint windowWidth, windowHeight;
		glfwGetFramebufferSize(win_ptr, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);

		// Draw stuff
		glClearColor(0.0, 0.3, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW_MATRIX);

		gluLookAt(eye.x(),eye.y(),eye.z(),  // eye  
		gaze.x(), gaze.y(), gaze.z(),      // center  
		0.0, 1.0, 0.);      // up direction

		for(const auto obj : objects){
			obj.second->display();
		}

		glMatrixMode(GL_PROJECTION_MATRIX);
		glLoadIdentity();



		gluPerspective( 90, (double)windowHeight / (double)windowWidth, .01, 100 );



		// Update Screen
		glfwSwapBuffers(win_ptr);
		//keyboard_control();
		return true;

	}else{
		Manager::windows.erase(name_);
		for(const auto& obj : objects){
			obj.second->windows.erase(name_);
		}
		glfwDestroyWindow(win_ptr);
		win_ptr=NULL;
		return false;
	}


}

void ObjectWindow::add_object(Object* obj){
	objects[obj->name_] = obj;
	obj->windows[name_] = this;
}
	



Object::Object(std::string name):
pose(Eigen::Affine3d::Identity()),
draw(true),
name_(name){
}

Object::~Object(){
	del();
}

void Object::del(){
	for(const auto& win : windows ){
		win.second->objects.erase(name_);
	}
}

void Object::display(){
	glPushMatrix();
	Eigen::Matrix4d scene_mat;
	glGetDoublev(GL_MODELVIEW_MATRIX, scene_mat.data());
	scene_mat = scene_mat*pose.matrix();
	glLoadMatrixd(scene_mat.data());
	draw_obj();
	glPopMatrix();
}

void Object::draw_obj(){

}

void Object::set_transform(Eigen::Affine3d t){
	pose = t;
}

void Object::translate(Eigen::Translation3d t){
	pose = t*pose;
}

void Object::rotate(Eigen::Quaterniond q){
	pose = q*pose;
}

void Cube::draw_obj(){

    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);

    // top
    glColor3f(1.0f, 0.0f, 0.0f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-hWidth, hHeight, hLength);
    glVertex3f(hWidth, hHeight, hLength);
    glVertex3f(hWidth, hHeight, -hLength);
    glVertex3f(-hWidth, hHeight, -hLength);
 
    // front
    glColor3f(0.0f, 1.0f, 0.0f);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(hWidth, -hHeight, hLength);
    glVertex3f(hWidth, hHeight, hLength);
    glVertex3f(-hWidth, hHeight, hLength);
    glVertex3f(-hWidth, -hHeight, hLength);
 
    // right
    glColor3f(0.0f, 0.0f, 1.0f);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(hWidth, hHeight, -hLength);
    glVertex3f(hWidth, hHeight, hLength);
    glVertex3f(hWidth, -hHeight, hLength);
    glVertex3f(hWidth, -hHeight, -hLength);
 
    // left
    glColor3f(0.0f, 0.0f, 0.5f);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-hWidth, -hHeight, hLength);
    glVertex3f(-hWidth, hHeight, hLength);
    glVertex3f(-hWidth, hHeight, -hLength);
    glVertex3f(-hWidth, -hHeight, -hLength);

    // bottom
    glColor3f(0.5f, 0.0f, 0.0f);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-hWidth, -hHeight, hLength);
    glVertex3f(hWidth, -hHeight, hLength);
    glVertex3f(hWidth, -hHeight, -hLength);
    glVertex3f(-hWidth, -hHeight, -hLength);
 
    // back
    glColor3f(0.0f, 0.5f, 0.0f);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(hWidth, hHeight, -hLength);
    glVertex3f(hWidth, -hHeight, -hLength);
    glVertex3f(-hWidth, -hHeight, -hLength);
    glVertex3f(-hWidth, hHeight, -hLength);
 
    glEnd();

    glEnable(GL_LIGHTING);

}


void Grid::draw_obj()
{
    // disable lighting
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);

    glColor3f(0.3f, 0.3f, 0.3f);
    for(float i=step; i <= size; i+= step)
    {
        glVertex3f(-size, 0,  i);   // lines parallel to X-axis
        glVertex3f( size, 0,  i);
        glVertex3f(-size, 0, -i);   // lines parallel to X-axis
        glVertex3f( size, 0, -i);

        glVertex3f( i, 0, -size);   // lines parallel to Z-axis
        glVertex3f( i, 0,  size);
        glVertex3f(-i, 0, -size);   // lines parallel to Z-axis
        glVertex3f(-i, 0,  size);
    }

    // x-axis
    glColor3f(0.5f, 0, 0);
    glVertex3f(-size, 0, 0);
    glVertex3f( size, 0, 0);

    // z-axis
    glColor3f(0,0,0.5f);
    glVertex3f(0, 0, -size);
    glVertex3f(0, 0,  size);

    glEnd();

    // enable lighting back
    glEnable(GL_LIGHTING);
}


void Axis::draw_obj()
{
    glDepthFunc(GL_ALWAYS);   
    glDisable(GL_LIGHTING);
    glPushMatrix();             

    // draw axis
    glLineWidth(3);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(size, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, size, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, size);
    glEnd();
    glLineWidth(1);

    // draw endpoints
    glPointSize(5);
    glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        glVertex3f(size, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, size, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, size);
    glEnd();
    glPointSize(1);

    // restore default settings
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glDepthFunc(GL_LEQUAL);
}

void Path::draw_obj()
{
    // disable lighting
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);

    glColor3f(color.x(),color.y(),color.z());
    for(int i =1; i<nodes.size(); i++)
    {
        glVertex3f(nodes[i-1].x(),nodes[i-1].y(),nodes[i-1].z());   // lines parallel to X-axis
        glVertex3f(nodes[i].x(),nodes[i].y(),nodes[i].z());   // lines parallel to X-axis
    }

    glEnd();

    // enable lighting back
    glEnable(GL_LIGHTING);
}

}//MyGUI

