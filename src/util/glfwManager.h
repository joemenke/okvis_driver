#ifdef _WIN32
	#include <Windows.h>
	#include <gl/GLU.h>
#else
	#include <GL/glew.h>
#endif
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <map>
#include <vector>

#include <opencv2/core/core.hpp>


namespace MyGUI{



class Window;
class Object;

class Manager {
public:
	static std::map<std::string,Window*> windows;

	//This function simply references GLFW init
	static int (&init)(); 

	static void (&terminate)(); 

	static bool running();

	static void update();

};//Manager

class Window{
public:
	GLFWwindow* win_ptr;
	std::string name_;

	Window(std::string name, int resX, int resY);

	virtual ~Window();

	virtual bool display();

	void add_control_func(GLFWkeyfun controls);

	virtual void keyboard_control()
	{
	    if(glfwGetKey(win_ptr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	            glfwSetWindowShouldClose(win_ptr, GL_TRUE);
	}

};

class ObjectWindow : public Window{
public:
	std::map<std::string,Object*> objects;

	Eigen::Vector3d eye;
	Eigen::Vector3d gaze;

	ObjectWindow(std::string name, int resX, int resY);

	virtual ~ObjectWindow();

	virtual bool display();

	void add_object(Object* obj);

	void set_camera(Eigen::Vector3d eye, Eigen::Vector3d gaze);


};//Window


class CameraWindow : public ObjectWindow{

public:

	CameraWindow(std::string name, int resX, int resY):
	ObjectWindow(name,resX,resY){}
	

	void keyboard_control()
	{
    float cameraSpeed = 1.0f; // adjust accordingly
    if(glfwGetKey(win_ptr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win_ptr, GL_TRUE);
    if(glfwGetKey(win_ptr, GLFW_KEY_W) == GLFW_PRESS)
        eye += cameraSpeed * Eigen::Vector3d(0,1,0);
    if(glfwGetKey(win_ptr, GLFW_KEY_S) == GLFW_PRESS)
        eye -= cameraSpeed * Eigen::Vector3d(0,1,0);
    if(glfwGetKey(win_ptr, GLFW_KEY_A) == GLFW_PRESS)
    	eye -= ((gaze-eye).normalized().cross(Eigen::Vector3d(0,1,0))).normalized() * cameraSpeed;
    if(glfwGetKey(win_ptr, GLFW_KEY_D) == GLFW_PRESS)
    	eye += ((gaze-eye).normalized().cross(Eigen::Vector3d(0,1,0))).normalized() * cameraSpeed;
    if(glfwGetKey(win_ptr, GLFW_KEY_Z) == GLFW_PRESS)
        eye += cameraSpeed * (gaze-eye).normalized();
    if(glfwGetKey(win_ptr, GLFW_KEY_X) == GLFW_PRESS)
        eye -= cameraSpeed * (gaze-eye).normalized();
	}
};

class ImageWindow : public Window{
public:
	std::shared_ptr< cv::Mat> current_image;
	GLuint texture;
	GLenum image_format_;
	GLenum data_type_;

	//Image format examples: GL_RGB, GLBGR, GL_LUMINANCE
	//Data type examples GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT
	ImageWindow(std::string name, int resX, int resY, GLenum image_format, GLenum data_type):
	Window(name,resX,resY),
	image_format_(image_format),
	data_type_(data_type){
		glGenTextures(1,&texture);
	}


	void set_image(std::shared_ptr<cv::Mat> image_in);

	bool display();

};


class Object{
public:
	std::map<std::string,ObjectWindow*> windows;
	Eigen::Affine3d pose;
	bool draw;
	std::string name_;

	Object(std::string name);

	virtual ~Object();

	void del();

	void display();

	void set_transform(Eigen::Affine3d t);

	void translate(Eigen::Translation3d t);

	void rotate(Eigen::Quaterniond q);

	virtual void draw_obj();

};//Object

class Cube : public Object {
public:
	float hWidth, hHeight, hLength;
	void draw_obj();

	Cube(std::string name, float width, float height, float length) : 
	Object(name), hWidth(width/2), hHeight(height/2), hLength(length/2){

	}
};

class Grid : public Object {
public:
	float size, step;
	void draw_obj();

	Grid(std::string name, float size, float step) 
	: Object(name),
	size(size),
	step(step){

	}
};

class Axis : public Object {
public:
	float size;
	void draw_obj();

	Axis(std::string name, float size) 
	: Object(name),
	size(size){

	}

};

class Path : public Object {
public:
	std::vector<Eigen::Vector3d> nodes;
	Eigen::Vector3d color;
	void draw_obj();

	Path(std::string name)
	: Object(name),
	nodes(){
	}

	Path(std::string name, Eigen::Vector3d color)
	: Object(name),
	nodes(),
	color(color){
	}

	Path(std::string name, const std::vector<Eigen::Vector3d>& nodes)
	: Object(name),
	nodes(nodes){
	}

	void add_node(Eigen::Vector3d node){
		nodes.push_back(node);
	}

	void set_color(Eigen::Vector3d obj_color){
		color = obj_color;
	}

	void set_color(float x, float y, float z){
		color = Eigen::Vector3d(x,y,z);
	}
};








} //MyGUI
