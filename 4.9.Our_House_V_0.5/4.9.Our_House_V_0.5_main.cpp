#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.

#define CAM_NUM 10
#define CAM_TRANSLATION_SPEED 0.025f
#define CAM_ROTATION_SPEED 0.1f

enum VIEW {
	TOP_VIEW = 0, FRONT_VIEW, SIDE_VIEW, CCTV1_VIEW, CCTV2_VIEW, CCTV3_VIEW, MAIN_VIEW, DYNAMIC_CCTV_VIEW
}VIEW;

typedef struct _CAMERA {
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;
	float fov_y, aspect_ratio, near_clip, far_clip;
	int move_status;
}CAMERA;

CAMERA cam[CAM_NUM];
int cam_selected = MAIN_VIEW;


int move_mode = 1; //0:translation mode 1: roatation mode
int CCTV_selected=1;

typedef struct _VIEWPORT {
	int x, y, w, h;
}VIEWPORT;

VIEWPORT viewport[CAM_NUM];

glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ModelViewMatrix[CAM_NUM], ViewMatrix[CAM_NUM], ProjectionMatrix[CAM_NUM], ViewProjectionMatrix[CAM_NUM], ModelMatrix[CAM_NUM];

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

#include "Object_Definitions.h"

void set_ViewMatrix(int cam_idx) {

	ViewMatrix[cam_idx] = glm::mat4(1.0f);
	ViewMatrix[cam_idx][0].x = cam[cam_idx].uaxis.x;
	ViewMatrix[cam_idx][0].y = cam[cam_idx].vaxis.x;
	ViewMatrix[cam_idx][0].z = cam[cam_idx].naxis.x;

	ViewMatrix[cam_idx][1].x = cam[cam_idx].uaxis.y;
	ViewMatrix[cam_idx][1].y = cam[cam_idx].vaxis.y;
	ViewMatrix[cam_idx][1].z = cam[cam_idx].naxis.y;

	ViewMatrix[cam_idx][2].x = cam[cam_idx].uaxis.z;
	ViewMatrix[cam_idx][2].y = cam[cam_idx].vaxis.z;
	ViewMatrix[cam_idx][2].z = cam[cam_idx].naxis.z;

	ViewMatrix[cam_idx] = glm::translate(ViewMatrix[cam_idx], -cam[cam_idx].pos);
}

typedef struct _CALLBACK_CONTEXT {
	int prevx, prevy, left_button_status, right_button_status;
	float rotation_angle_cow;
}CALLBACK_CONTEXT;
CALLBACK_CONTEXT cc;

void display_camera(int cam_idx) {

	glViewport(viewport[cam_idx].x, viewport[cam_idx].y, viewport[cam_idx].w, viewport[cam_idx].h);
	
	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[cam_idx], glm::vec3(5.0f, 5.0f, 5.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glLineWidth(2.0f);
	draw_axes(cam_idx);
	glLineWidth(1.0f);

	draw_static_object(&(static_objects[OBJ_BUILDING]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_MINIBUILDING]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_TABLE]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_TABLE]), 1, cam_idx);

	draw_static_object(&(static_objects[OBJ_LIGHT]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 1, cam_idx);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 2, cam_idx);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 3, cam_idx);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 4, cam_idx);

	draw_static_object(&(static_objects[OBJ_TEAPOT1]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_TEAPOT2]), 0, cam_idx);

	draw_static_object(&(static_objects[OBJ_NEW_CHAIR1]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_NEW_CHAIR2]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_FRAME]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_NEW_PICTURE]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_COW1]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_COW2]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_GODZILLA]), 0, cam_idx);
	draw_static_object(&(static_objects[OBJ_IRONMAN]), 0, cam_idx);
	display_view(cam_idx);
	display_cctv(cam_idx);
	draw_main_cam(cam_idx);
	draw_animated_tiger(cam_idx);
	display_car(cam_idx);

}

void renew_cam_position(int cam_idx, float del, glm::vec3 trans_axis) {
	cam[cam_idx].pos += CAM_TRANSLATION_SPEED * del * trans_axis;
}

void renew_cam_rotation(int cam_idx, float angle, glm::vec3 rot_axis) {
	
	glm::mat3 RotationMatrix;

	RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), CAM_ROTATION_SPEED*TO_RADIAN*angle, rot_axis));

	cam[cam_idx].uaxis = RotationMatrix * cam[cam_idx].uaxis;
	cam[cam_idx].vaxis = RotationMatrix * cam[cam_idx].vaxis;
	cam[cam_idx].naxis = RotationMatrix * cam[cam_idx].naxis;

}

void motion(int x, int y) {

	if (!cam[cam_selected].move_status) return;
	float dx = (float)(x - cc.prevx);
	float dy = (float)(cc.prevy - y);
	cc.prevx = x; cc.prevy = y;

	if (cam_selected == MAIN_VIEW) {
		switch (move_mode) {
			case 0: //rotation mode
				if (cc.left_button_status == GLUT_DOWN) {
					renew_cam_rotation(cam_selected, -dy, -cam[cam_selected].uaxis);
					renew_cam_rotation(cam_selected, dx, -cam[cam_selected].vaxis);
				}
				else if(cc.right_button_status == GLUT_DOWN) {
					renew_cam_rotation(cam_selected, 2*dx, -cam[cam_selected].naxis);
				}
				break;
			case 1:	//trainslation mode
				if (cc.left_button_status == GLUT_DOWN) {
					renew_cam_position(cam_selected, -10*dx, cam[cam_selected].uaxis);
					renew_cam_position(cam_selected, -10*dy, cam[cam_selected].vaxis);
				}
				else if (cc.right_button_status == GLUT_DOWN) {
					renew_cam_position(cam_selected, -dy*10 , cam[cam_selected].naxis);
				}
				break;
		}
		
	}

	if (cam_selected == DYNAMIC_CCTV_VIEW) {
		if (cc.left_button_status == GLUT_DOWN) {
			renew_cam_rotation(cam_selected, -dy, -cam[cam_selected].uaxis);
			renew_cam_rotation(cam_selected, dx, -cam[cam_selected].vaxis);
		}
		if (cc.right_button_status == GLUT_DOWN) {
			renew_cam_rotation(cam_selected, 2 * dx, -cam[cam_selected].naxis);
		}

	}
	set_ViewMatrix(cam_selected);
	ViewProjectionMatrix[cam_selected] = ProjectionMatrix[cam_selected] * ViewMatrix[cam_selected];
	

	glutPostRedisplay();

}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	display_camera(TOP_VIEW);
	display_camera(FRONT_VIEW);
	display_camera(SIDE_VIEW);
	
	switch (CCTV_selected) {
		case 1:
			display_camera(CCTV1_VIEW);
			break;
		case 2:
			display_camera(CCTV2_VIEW);
			break;
		case 3:
			display_camera(CCTV3_VIEW);
			break;
	}

	display_camera(cam_selected);
	glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 0, polygon_fill_on = 0, depth_test_on = 0;
	static int main_view_mode = 1;

	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'c':
		flag_cull_face = (flag_cull_face + 1) % 3;
		switch (flag_cull_face) {
		case 0:
			glDisable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ No faces are culled.\n");
			break;
		case 1: // cull back faces;
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Back faces are culled.\n");
			break;
		case 2: // cull front faces;
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Front faces are culled.\n");
			break;

		}
		break;
	case 'f':
		polygon_fill_on = 1 - polygon_fill_on;
		if (polygon_fill_on) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fprintf(stdout, "^^^ Polygon filling enabled.\n");
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			fprintf(stdout, "^^^ Line drawing enabled.\n");
		}
		glutPostRedisplay();
		break;
	case 'd':
		depth_test_on = 1 - depth_test_on;
		if (depth_test_on) {
			glEnable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test enabled.\n");
		}
		else {
			glDisable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test disabled.\n");
		}
		glutPostRedisplay();
		break;
	case '1':
		fprintf(stdout, "^^^ Show CCTV1.\n");
		CCTV_selected = 1;
		break;
	case '2':
		fprintf(stdout, "^^^ Show CCTV2.\n");
		CCTV_selected = 2;
		break;
	case '3':
		fprintf(stdout, "^^^ Show CCTV3.\n");
		CCTV_selected = 3;
		break;
	case 'm':
		if (main_view_mode == 0) {
			fprintf(stdout, "^^^ Main camera mode.\n");
			cam_selected = MAIN_VIEW;
			main_view_mode = 1;
		}
		else {
			fprintf(stdout, "^^^ Dynamic cctv mode.\n");
			cam_selected = DYNAMIC_CCTV_VIEW;
			main_view_mode = 0;
		}
		break;
	case't':
		move_mode++;
		move_mode %= 2;
		if (cam_selected == MAIN_VIEW) {
			if (move_mode == 0) fprintf(stdout, "^^^ Rotation mode.\n");
			else if (move_mode == 1) fprintf(stdout, "^^^ Translation mode.\n");
		}
		break;
	}
}
void mousepress(int button, int state, int x, int y) {
	//mouse right click and move
	if(button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			cc.left_button_status = GLUT_DOWN;
			cam[cam_selected].move_status = 1;
			cc.prevx = x; cc.prevy = y;
		}
		else if (state == GLUT_UP) {
			cc.left_button_status = GLUT_UP;
			cam[cam_selected].move_status = 0;
		}
	}

	//mouse left click and move
	if( button == GLUT_RIGHT_BUTTON ) {
		if (state == GLUT_DOWN) {
			cc.right_button_status = GLUT_DOWN;
			cam[cam_selected].move_status = 1;
			cc.prevx = x; cc.prevy = y;
		}
		else if (state == GLUT_UP) {
			cc.right_button_status = GLUT_UP;
			cam[cam_selected].move_status = 0;
		}
	}

	
	
}

void mouseWheel(int button, int dir, int x, int y)
{
	if (dir < 0) {
		if (cam[cam_selected].fov_y > 5*TO_RADIAN ){
			cam[cam_selected].fov_y += dir * 0.01;
			ProjectionMatrix[cam_selected] = glm::perspective(cam[cam_selected].fov_y, cam[cam_selected].aspect_ratio, cam[cam_selected].near_clip, cam[cam_selected].far_clip);
			ViewProjectionMatrix[cam_selected] = ProjectionMatrix[cam_selected] * ViewMatrix[cam_selected];

			glutPostRedisplay();
		}
	}
	if (dir > 0 ){ 
		if (cam[cam_selected].fov_y < 100*TO_RADIAN ) {
			cam[cam_selected].fov_y += dir * 0.01;	
			ProjectionMatrix[cam_selected] = glm::perspective(cam[cam_selected].fov_y, cam[cam_selected].aspect_ratio, cam[cam_selected].near_clip, cam[cam_selected].far_clip);
			ViewProjectionMatrix[cam_selected] = ProjectionMatrix[cam_selected] * ViewMatrix[cam_selected];

			glutPostRedisplay();
		}
	}

	
	return;
}



void reshape(int width, int height) {

	cam[TOP_VIEW].aspect_ratio = (float)width / height;
	viewport[TOP_VIEW].x = 0.01f*width;
	viewport[TOP_VIEW].y = 0.65f*height; //500.0f;
	viewport[TOP_VIEW].w = (int)(0.35f*width);
	viewport[TOP_VIEW].h = (int)(0.35f*height);
	ProjectionMatrix[TOP_VIEW] = glm::ortho(-cam[TOP_VIEW].pos.x, cam[TOP_VIEW].pos.x, -cam[TOP_VIEW].pos.y, cam[TOP_VIEW].pos.y, cam[TOP_VIEW].near_clip, cam[TOP_VIEW].far_clip);
	ViewProjectionMatrix[TOP_VIEW] = ProjectionMatrix[TOP_VIEW] * ViewMatrix[TOP_VIEW];

	//cam[FRONT_VIEW].aspect_ratio = cam[TOP_VIEW].aspect_ratio; 
	viewport[FRONT_VIEW].x = (int)(0.75f*width);
	viewport[FRONT_VIEW].y = (int)(0.79f*height);
	viewport[FRONT_VIEW].w = (int)(0.25f*width);
	viewport[FRONT_VIEW].h = (int)(0.20f*height);
	cam[FRONT_VIEW].aspect_ratio = viewport[FRONT_VIEW].w / viewport[FRONT_VIEW].h;
	ProjectionMatrix[FRONT_VIEW] = glm::ortho(-cam[FRONT_VIEW].pos.x, cam[FRONT_VIEW].pos.x, -cam[FRONT_VIEW].pos.z, cam[FRONT_VIEW].pos.z, cam[FRONT_VIEW].near_clip, cam[FRONT_VIEW].far_clip);
	ViewProjectionMatrix[FRONT_VIEW] = ProjectionMatrix[FRONT_VIEW] * ViewMatrix[FRONT_VIEW];


	cam[SIDE_VIEW].aspect_ratio = cam[FRONT_VIEW].aspect_ratio; 
	viewport[SIDE_VIEW].x = (int)(0.75f*width);
	viewport[SIDE_VIEW].y = (int)(0.58f*height);
	viewport[SIDE_VIEW].w = (int)(0.25f*width);
	viewport[SIDE_VIEW].h = (int)(0.20f*height);
	ProjectionMatrix[SIDE_VIEW] = glm::ortho(-cam[SIDE_VIEW].pos.y, cam[SIDE_VIEW].pos.y, -cam[SIDE_VIEW].pos.z, cam[SIDE_VIEW].pos.z, cam[SIDE_VIEW].near_clip, cam[SIDE_VIEW].far_clip);
	ViewProjectionMatrix[SIDE_VIEW] = ProjectionMatrix[SIDE_VIEW] * ViewMatrix[SIDE_VIEW];

	for (int cctv = CCTV1_VIEW; cctv <= CCTV3_VIEW; cctv++) {
		cam[cctv].aspect_ratio = cam[TOP_VIEW].aspect_ratio;
		viewport[cctv].x = (int)(0.35f*width);
		viewport[cctv].y = (int)(0.60f*height);
		viewport[cctv].w = (int)(0.40f*width);
		viewport[cctv].h = (int)(0.40f*height);
		ProjectionMatrix[cctv] = glm::perspective(cam[cctv].fov_y, cam[cctv].aspect_ratio, cam[cctv].near_clip, cam[cctv].far_clip);
		ViewProjectionMatrix[cctv] = ProjectionMatrix[cctv] * ViewMatrix[cctv];
	}

	viewport[MAIN_VIEW].x = (int)(0.05f*width);
	viewport[MAIN_VIEW].y = (int)(0.05f*height);
	viewport[MAIN_VIEW].w = (int)(0.9f*width);
	viewport[MAIN_VIEW].h = (int)(0.5f*height);
	cam[MAIN_VIEW].aspect_ratio = viewport[MAIN_VIEW].w / viewport[MAIN_VIEW].h;
	ProjectionMatrix[MAIN_VIEW] = glm::perspective(cam[MAIN_VIEW].fov_y, cam[MAIN_VIEW].aspect_ratio, cam[MAIN_VIEW].near_clip, cam[MAIN_VIEW].far_clip);
	ViewProjectionMatrix[MAIN_VIEW] = ProjectionMatrix[MAIN_VIEW] * ViewMatrix[MAIN_VIEW];


	viewport[DYNAMIC_CCTV_VIEW].x = (int)(0.05f*width);
	viewport[DYNAMIC_CCTV_VIEW].y = (int)(0.05f*height);
	viewport[DYNAMIC_CCTV_VIEW].w = (int)(0.9f*width);
	viewport[DYNAMIC_CCTV_VIEW].h = (int)(0.5f*height);
	cam[DYNAMIC_CCTV_VIEW].aspect_ratio = viewport[DYNAMIC_CCTV_VIEW].w / viewport[DYNAMIC_CCTV_VIEW].h;
	ProjectionMatrix[DYNAMIC_CCTV_VIEW] = glm::perspective(cam[DYNAMIC_CCTV_VIEW].fov_y, cam[DYNAMIC_CCTV_VIEW].aspect_ratio, cam[DYNAMIC_CCTV_VIEW].near_clip, cam[DYNAMIC_CCTV_VIEW].far_clip);
	ViewProjectionMatrix[DYNAMIC_CCTV_VIEW] = ProjectionMatrix[DYNAMIC_CCTV_VIEW] * ViewMatrix[DYNAMIC_CCTV_VIEW];


	glutPostRedisplay();
}

void timer_scene(int timestamp_scene) {
	rotation_angle_car = (timestamp_scene % 360)*TO_RADIAN;

	tiger_time = (timestamp_scene)%end_time;
	tiger_data.cur_frame = timestamp_scene % N_TIGER_FRAMES;
	tiger_data.rotation_angle = (timestamp_scene % 360)*TO_RADIAN;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mousepress);
	glutMouseWheelFunc(mouseWheel);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(10, timer_scene, 0);
	glutCloseFunc(cleanup_OpenGL_stuffs);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}



void initialize_camera() {
	//TOP_VIEW
	cam[TOP_VIEW].pos = glm::vec3(120.0f, 90.0f, 1000.0f);
	cam[TOP_VIEW].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	cam[TOP_VIEW].vaxis = glm::vec3(0.0f, 1.0f, 0.0f);
	cam[TOP_VIEW].naxis = glm::vec3(0.0f, 0.0f, 1.0f);

	cam[TOP_VIEW].move_status = 0;
	cam[TOP_VIEW].fov_y = 10.0f*TO_RADIAN;
	cam[TOP_VIEW].aspect_ratio = 1.0f;
	cam[TOP_VIEW].near_clip = 1.0f;	
	cam[TOP_VIEW].far_clip = 10000.0f;

	set_ViewMatrix(0);

	//FRONT_VIEW
	cam[FRONT_VIEW].pos = glm::vec3(120.0f, -1000.0f, 25.0f);
	cam[FRONT_VIEW].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	cam[FRONT_VIEW].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[FRONT_VIEW].naxis = glm::vec3(0.0f, -1.0f, 0.0f);

	cam[FRONT_VIEW].move_status = 0;
	cam[FRONT_VIEW].fov_y = 10.0f*TO_RADIAN;
	//cam[FRONT_VIEW].aspect_ratio = 1.0f;
	cam[FRONT_VIEW].near_clip = 1.0f;
	cam[FRONT_VIEW].far_clip = 10000.0f;


	set_ViewMatrix(1);

	//SIDE_VIEW
	cam[SIDE_VIEW].pos = glm::vec3(1000.0f, 90.0f, 25.0f);
	cam[SIDE_VIEW].uaxis = glm::vec3(0.0f, 1.0f, 0.0f);
	cam[SIDE_VIEW].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[SIDE_VIEW].naxis = glm::vec3(1.0f, 0.0f, 0.0f);

	cam[SIDE_VIEW].move_status = 0;
	cam[SIDE_VIEW].fov_y = 10.0f*TO_RADIAN;
	cam[SIDE_VIEW].near_clip = 1.0f;
	cam[SIDE_VIEW].far_clip = 10000.0f;

	set_ViewMatrix(2);

	//CCTV1
	cam[CCTV1_VIEW].pos = glm::vec3(225.0f, 160.0f, 30.0f);
	cam[CCTV1_VIEW].uaxis = glm::vec3(-0.5f, 0.5f, 0.0f);
	cam[CCTV1_VIEW].naxis = glm::vec3(0.5f, 0.5f, 0.0f);
	cam[CCTV1_VIEW].vaxis = glm::cross(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(-0.5f, -0.5f, 0.0f));

	cam[CCTV1_VIEW].move_status = 0;
	cam[CCTV1_VIEW].fov_y = 50.0f*TO_RADIAN;
	cam[CCTV1_VIEW].near_clip = 1.0f;
	cam[CCTV1_VIEW].far_clip = 10000.0f;
	set_ViewMatrix(CCTV1_VIEW);

	//cctv2
	cam[CCTV2_VIEW].pos = glm::vec3(15.0f, 100.0f, 30.0f);
	cam[CCTV2_VIEW].uaxis = glm::vec3(0.0f, -1.0f, 0.0f);
	cam[CCTV2_VIEW].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[CCTV2_VIEW].naxis = glm::vec3(-1.0f, 0.0f, 0.0f);

	cam[CCTV2_VIEW].move_status = 0;
	cam[CCTV2_VIEW].fov_y = 50.0f*TO_RADIAN;
	cam[CCTV2_VIEW].near_clip = 1.0f;
	cam[CCTV2_VIEW].far_clip = 10000.0f;
	set_ViewMatrix(CCTV2_VIEW);


	//cctv3
	cam[CCTV3_VIEW].pos = glm::vec3(120.0f, 55.0f, 30.0f);
	cam[CCTV3_VIEW].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	cam[CCTV3_VIEW].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[CCTV3_VIEW].naxis = glm::vec3(0.0f, -1.0f, 0.0f);

	cam[CCTV3_VIEW].move_status = 0;
	cam[CCTV3_VIEW].fov_y = 50.0f*TO_RADIAN;
	cam[CCTV3_VIEW].near_clip = 1.0f;
	cam[CCTV3_VIEW].far_clip = 10000.0f;
	set_ViewMatrix(CCTV3_VIEW);


	cam[MAIN_VIEW].pos = glm::vec3(600.0f, 600.0f, 200.0f);
	cam[MAIN_VIEW].naxis = glm::normalize(glm::vec3(600.0f - 125.0f, 600.0f - 80.0f, 200.0f - 25.0f));
	cam[MAIN_VIEW].uaxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), cam[MAIN_VIEW].naxis);
	cam[MAIN_VIEW].vaxis = glm::cross(cam[MAIN_VIEW].uaxis,-cam[MAIN_VIEW].naxis);
	

	cam[MAIN_VIEW].move_status = 0;
	cam[MAIN_VIEW].fov_y = 14.0f*TO_RADIAN;
	cam[MAIN_VIEW].near_clip = 1.0f;
	cam[MAIN_VIEW].far_clip = 1000.0f;
	set_ViewMatrix(MAIN_VIEW);

	cam[DYNAMIC_CCTV_VIEW].pos = glm::vec3(200.0f, 40.0f, 50.0f);
	cam[DYNAMIC_CCTV_VIEW].naxis = glm::normalize(glm::vec3(200.0f - 200.0f, 40.0f - 135.0f, 50.0f - 25.0f));
	cam[DYNAMIC_CCTV_VIEW].uaxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), cam[DYNAMIC_CCTV_VIEW].naxis);
	cam[DYNAMIC_CCTV_VIEW].vaxis = glm::cross(cam[DYNAMIC_CCTV_VIEW].uaxis, -cam[DYNAMIC_CCTV_VIEW].naxis);


	cam[DYNAMIC_CCTV_VIEW].move_status = 0;
	cam[DYNAMIC_CCTV_VIEW].fov_y = 80.0f*TO_RADIAN;
	cam[DYNAMIC_CCTV_VIEW].near_clip = 1.0f;
	cam[DYNAMIC_CCTV_VIEW].far_clip = 10000.0f;

	cc.left_button_status = GLUT_UP;
	cc.left_button_status = GLUT_UP;
	set_ViewMatrix(DYNAMIC_CCTV_VIEW);
}


void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST); // Default state
	 
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.12f, 0.18f, 0.12f, 1.0f);
	initialize_camera();
}

void prepare_scene(void) {
	char car[][50] = { "Data/car_body_triangles_v.txt" ,"Data/car_wheel_triangles_v.txt","Data/car_nut_triangles_v.txt" };
	define_axes();
	define_static_objects();
	prepare_cctv();
	prepare_view();
	prepare_path();
	define_animated_tiger();

	prepare_path();
	prepare_geom_obj(GEOM_OBJ_ID_CAR_BODY, car[0], GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_WHEEL,car[1] , GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_NUT, car[2], GEOM_OBJ_TYPE_V);
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) { 
	char program_name[256] = "Sogang CSE4170 Our_House_GLSL_V_0.5";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'c', 'f', 'd', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(3, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
