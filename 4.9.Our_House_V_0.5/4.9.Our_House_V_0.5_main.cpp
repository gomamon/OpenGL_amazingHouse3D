#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>

#include "Shaders/LoadShaders.h"
#include "My_shader.h"
GLuint h_ShaderProgram, h_ShaderProgram_PS, h_ShaderProgram_GS; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

#define NUMBER_OF_LIGHT_SUPPORTED 4
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
//-------
GLint loc_wall_effect, loc_wall_width;
GLint loc_blind_effect, loc_blind_extent, loc_my_blind_effect, loc_my_blind_extent, loc_cartoon_effect, loc_cartoon_levels;
//-------
GLint loc_ModelViewProjectionMatrix_PS, loc_ModelViewMatrix_PS, loc_ModelViewMatrixInvTrans_PS;
GLint loc_ModelViewProjectionMatrix_GS, loc_ModelViewMatrix_GS, loc_ModelViewMatrixInvTrans_GS;
float light_time = 0.0f;
int light_effect = 0;
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];
int light_power[NUMBER_OF_LIGHT_SUPPORTED];
// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp>	//inverseTranspose
#define CAM_NUM 10
#define CAM_TRANSLATION_SPEED 0.025f
#define CAM_ROTATION_SPEED 0.1f


enum SHADER {
	PS=0, GS
}SHADER;
int shader_mode = PS;

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
glm::mat3 ModelViewMatrixInvTrans;
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


void set_up_scene_lights(int cam_idx) {
	
	light[0].light_on = light_power[0];
	light[0].position[0] = 0.0f; light[0].position[1] = 0.0f; 	// point light position in EC
	light[0].position[2] = 0.0f; light[0].position[3] = 1.0f;


	if(light_effect){
		light[0].ambient_color[0] = 0.2f + 0.1f*light_time+light_time/2; 
		light[0].ambient_color[1] = 0.3f + 0.25f*light_time;
		light[0].ambient_color[2] = light_time/2 - 0.08f*light_time; 
		light[0].ambient_color[3] = 1.0f;

		light[0].diffuse_color[0] = 0.8f; light[0].diffuse_color[1] = 0.8f;
		light[0].diffuse_color[2] = 0.8f; light[0].diffuse_color[3] = 1.0f;

		light[0].specular_color[0] = 0.8f; light[0].specular_color[1] = 0.8f;
		light[0].specular_color[2] = 0.8f; light[0].specular_color[3] = 1.0f;
	}
	else {
		light[0].ambient_color[0] = 1.0f; light[0].ambient_color[1] = 1.0f;
		light[0].ambient_color[2] = 1.0f; light[0].ambient_color[3] = 1.0f;

		light[0].diffuse_color[0] = 0.8f; light[0].diffuse_color[1] = 0.8f;
		light[0].diffuse_color[2] = 0.8f; light[0].diffuse_color[3] = 1.0f;

		light[0].specular_color[0] = 0.8f; light[0].specular_color[1] = 0.8f;
		light[0].specular_color[2] = 0.8f; light[0].specular_color[3] = 1.0f;
	}

	// glm::vec3(210.0f, 112.5f, 49.0)
	// spot_light_WC: use light 1
	light[1].light_on = light_power[1];
	light[1].position[0] = 210.0f; light[1].position[1] = 112.5f; // spot light position in WC
	light[1].position[2] = 49.0f; light[1].position[3] = 1.0f;

	light[1].ambient_color[0] = 0.2f; light[1].ambient_color[1] = 0.2f;
	light[1].ambient_color[2] = 0.2f; light[1].ambient_color[3] = 1.0f;

	light[1].diffuse_color[0] = 0.82f; light[1].diffuse_color[1] = 0.82f;
	light[1].diffuse_color[2] = 0.82f; light[1].diffuse_color[3] = 1.0f;

	light[1].specular_color[0] = 0.82f; light[1].specular_color[1] = 0.82f;
	light[1].specular_color[2] = 0.82f; light[1].specular_color[3] = 1.0f;

	light[1].spot_direction[0] = 0.0f; light[1].spot_direction[1] = 0.0f; // spot light direction in WC
	light[1].spot_direction[2] = -1.0f;
	light[1].spot_cutoff_angle = 40.0f;
	light[1].spot_exponent = 20.0f;

	
	//spot light
	//40.0f, 87.0f, 5.0f
	light[2].light_on = light_power[2];
	light[2].position[0] = 40.0f; light[2].position[1] = 87.0f; // spot light position in WC
	light[2].position[2] = 50.0f; light[2].position[3] = 1.0f;

	light[2].ambient_color[0] = 0.2f; light[2].ambient_color[1] = 0.2f;
	light[2].ambient_color[2] = 0.2f; light[2].ambient_color[3] = 1.0f;

	light[2].diffuse_color[0] = 0.82f; light[2].diffuse_color[1] = 0.82f;
	light[2].diffuse_color[2] = 0.82f; light[2].diffuse_color[3] = 1.0f;

	light[2].specular_color[0] = 0.82f; light[2].specular_color[1] = 0.82f;
	light[2].specular_color[2] = 0.82f; light[2].specular_color[3] = 1.0f;

	light[2].spot_direction[0] = 0.0f; light[2].spot_direction[1] = 0.0f; // spot light direction in WC
	light[2].spot_direction[2] = -1.0f;
	light[2].spot_cutoff_angle = 40.0f;
	light[2].spot_exponent = 20.0f;


	if(shader_mode==GS)		glUseProgram(h_ShaderProgram_GS);
	else	glUseProgram(h_ShaderProgram_PS);
//	glUniform1i(loc_blind_effect, flag_blind_effect);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);


	for (int i = 1; i <= 2; i++) {
	//	glUniform1i(loc_blind_effect, 1);
		glUniform1i(loc_light[i].light_on, light[i].light_on);
		// need to supply position in EC for shading
		glm::vec4 position_EC = ViewMatrix[cam_idx] * glm::vec4(light[i].position[0], light[i].position[1],
			light[i].position[2], light[i].position[3]);
		glUniform4fv(loc_light[i].position, 1, &position_EC[0]);
		glUniform4fv(loc_light[i].ambient_color, 1, light[i].ambient_color);
		glUniform4fv(loc_light[i].diffuse_color, 1, light[i].diffuse_color);
		glUniform4fv(loc_light[i].specular_color, 1, light[i].specular_color);
		// need to supply direction in EC for shading in this example shader
		// note that the viewing transform is a rigid body transform
		// thus transpose(inverse(mat3(ViewMatrix)) = mat3(ViewMatrix)
		glm::vec3 direction_EC = glm::mat3(ViewMatrix[cam_idx]) * glm::vec3(light[i].spot_direction[0], light[i].spot_direction[1],
			light[i].spot_direction[2]);
		glUniform3fv(loc_light[i].spot_direction, 1, &direction_EC[0]);
		glUniform1f(loc_light[i].spot_cutoff_angle, light[i].spot_cutoff_angle);
		glUniform1f(loc_light[i].spot_exponent, light[i].spot_exponent);
	}
	glUseProgram(0);

}


void display_camera(int cam_idx) {
	set_up_scene_lights(cam_idx);

	glUseProgram(h_ShaderProgram);
	glViewport(viewport[cam_idx].x, viewport[cam_idx].y, viewport[cam_idx].w, viewport[cam_idx].h);
	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[cam_idx], glm::vec3(5.0f, 5.0f, 5.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);


	glLineWidth(2.0f);
	draw_axes(cam_idx);
	glLineWidth(1.0f);

	draw_main_cam(cam_idx);

	if(shader_mode==GS)		glUseProgram(h_ShaderProgram_GS);
	else		glUseProgram(h_ShaderProgram_PS);

	
	
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

	if(flag_draw_wall)
		draw_wall(cam_idx);

	draw_animated_tiger(cam_idx);
	display_car(cam_idx);
	//bbbbbbbbb

	//set_up_scene_lights(cam_idx);
	glUseProgram(0);

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
			//	else if (cc.right_button_status == GLUT_DOWN) {
				//	renew_cam_position(cam_selected, -dy*10 , cam[cam_selected].naxis);
				//}
				break;
		}
		
	}

	if (cam_selected == DYNAMIC_CCTV_VIEW) {
		if (cc.left_button_status == GLUT_DOWN) {
			renew_cam_rotation(cam_selected, -dy, -cam[cam_selected].uaxis);
			renew_cam_rotation(cam_selected, dx, -cam[cam_selected].vaxis);
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
void initialize_lights_and_material(void) {
	int i;

	
	if (shader_mode == GS) glUseProgram(h_ShaderProgram_GS);
	else glUseProgram(h_ShaderProgram_PS);

	glUniform4f(loc_global_ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}

	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]

	glUseProgram(0);


}

void update_color() {
	int i;
	char string[256];
	if (shader_mode == PS) {
		loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_global_ambient_color");
		for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
			sprintf(string, "u_light[%d].light_on", i);
			loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].position", i);
			loc_light[i].position = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].ambient_color", i);
			loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].diffuse_color", i);
			loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].specular_color", i);
			loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].spot_direction", i);
			loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].spot_exponent", i);
			loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].spot_cutoff_angle", i);
			loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_PS, string);
			sprintf(string, "u_light[%d].light_attenuation_factors", i);
			loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_PS, string);
		}

		loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.ambient_color");
		loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.diffuse_color");
		loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_color");
		loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.emissive_color");
		loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_exponent");
		



		glUseProgram(0);

	}
	else {
		loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_global_ambient_color");
		for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
			sprintf(string, "u_light[%d].light_on", i);
			loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].position", i);
			loc_light[i].position = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].ambient_color", i);
			loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].diffuse_color", i);
			loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].specular_color", i);
			loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].spot_direction", i);
			loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].spot_exponent", i);
			loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].spot_cutoff_angle", i);
			loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GS, string);
			sprintf(string, "u_light[%d].light_attenuation_factors", i);
			loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GS, string);
		}

		loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.ambient_color");
		loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.diffuse_color");
		loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_color");
		loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.emissive_color");
		loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_exponent");
	}
	for (int i = 0; i < CAM_NUM; i++) {
		set_up_scene_lights(i);
	}
	initialize_lights_and_material();

	
}
void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 1, polygon_fill_on = 0, depth_test_on = 0;
	static int main_view_mode = 1;
	//===========================
	if((((key >= '0') && (key <= '0' + NUMBER_OF_LIGHT_SUPPORTED - 1)))){
		int light_ID = (int)(key - '0');
		light_power[light_ID] = 1 - light_power[light_ID];
		return;
	}
	//===========================
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
	case 'l':
		light_effect = 1 - light_effect;
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
	//cctv
	case '!':
		fprintf(stdout, "^^^ Show CCTV1.\n");
		CCTV_selected = 1;
		break;
	case '@':
		fprintf(stdout, "^^^ Show CCTV2.\n");
		CCTV_selected = 2;
		break;
	case '#':
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
	//--------------------------------------------
	case'w':
		flag_draw_wall = 1 - flag_draw_wall;
		glutPostRedisplay();
		break;

	case 'e':
		if (flag_draw_wall) {
			flag_wall_effect = 1 - flag_wall_effect;
			glutPostRedisplay();
		}
		break;
	case '[':
		if(flag_draw_wall) {
			wall_width += 0.05f;
			if(wall_width >= 0.5)
				wall_width = 0.1;
			glUseProgram(h_ShaderProgram_PS);
			glUniform1f(loc_wall_width, wall_width);
			glutPostRedisplay();
		}
		break;
	case ']':
		if(flag_draw_wall) {
			wall_width -= 0.05f;
			if(wall_width <= 0.1f)
				wall_width = 0.5f;
			glUseProgram(h_ShaderProgram_PS);
			glUniform1f(loc_wall_width, wall_width);
			glutPostRedisplay();
		}
		break;
	case 'b':
		flag_blind_effect = 1 - flag_blind_effect;
		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_blind_effect, flag_blind_effect);
		glUseProgram(0);
	//	initialize_lights_and_material();
		glutPostRedisplay();		
		break;
	case 'n':
		if(flag_blind_effect) {
			flag_blind_effect = 0;
			glUseProgram(h_ShaderProgram_PS);
			glUniform1i(loc_blind_effect, flag_blind_effect);
			glUseProgram(0);
			glutPostRedisplay();
		}
		flag_my_blind_effect = 1 - flag_my_blind_effect;
		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_my_blind_effect, flag_my_blind_effect);
		glUseProgram(0);
		//	initialize_lights_and_material();
		glutPostRedisplay();
		break;
	case '+':
		if (flag_blind_effect) {
			blind_extent += 10.0f;
			if (blind_extent > 90)
				blind_extent = -90;
			glUseProgram(h_ShaderProgram_PS);
			glUniform1f(loc_blind_extent, blind_extent);
			glUseProgram(0);
			glutPostRedisplay();
		}
		else if(flag_my_blind_effect) {
			my_blind_extent += 10.0f;
			if(my_blind_extent > 90)
				my_blind_extent = -90;
			glUseProgram(h_ShaderProgram_PS);
			glUniform1f(loc_my_blind_extent, my_blind_extent);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	case '-':
		if (flag_blind_effect) {
			blind_extent -= 10.0f;
			if (blind_extent < -90)
				blind_extent = 90;
			glUseProgram(h_ShaderProgram_PS);
			glUniform1f(loc_blind_extent, blind_extent);
			glUseProgram(0);
			glutPostRedisplay();
		}
		else if(flag_my_blind_effect) {
			my_blind_extent -= 10.0f;
			if(my_blind_extent < -90)
				my_blind_extent = 90;
			glUseProgram(h_ShaderProgram_PS);
			glUniform1f(loc_my_blind_extent, my_blind_extent);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	case 'x':
		flag_cartoon_effect = 1 - flag_cartoon_effect;
		glUseProgram(h_ShaderProgram_PS);
		glUniform1f(loc_cartoon_levels, cartoon_levels);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 'z':
		if (flag_cartoon_effect) {
			if (cartoon_levels >= 3.0f) {
				cartoon_levels -= 1.0f;
				glUseProgram(h_ShaderProgram_PS);
				glUniform1f(loc_cartoon_levels, cartoon_levels);
				glUseProgram(0);
				glutPostRedisplay();
			}
		}
		break;
	case 'a':
		if (flag_cartoon_effect) {
			if (cartoon_levels <= 9.0f) {
				cartoon_levels += 1.0f;
				glUseProgram(h_ShaderProgram_PS);
				glUniform1f(loc_cartoon_levels, cartoon_levels);
				glUseProgram(0);
				glutPostRedisplay();
			}
		}
		break;
		//-----------------------------------------
	}

}
void mousepress(int button, int state, int x, int y) {
	//mouse right click and move
	if(button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
				move_mode = 0;
			}
			cc.left_button_status = GLUT_DOWN;
			cam[cam_selected].move_status = 1;
			cc.prevx = x; cc.prevy = y;
		}
		else if (state == GLUT_UP) {
			move_mode = 1;
			cc.left_button_status = GLUT_UP;
			cam[cam_selected].move_status = 0;
		}
	}

	//mouse left click and move
	if( button == GLUT_RIGHT_BUTTON ) {
		if (state == GLUT_DOWN) {
			if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
				shader_mode = GS;
				update_color();
			}
			else if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
				move_mode = 0;
				cam[cam_selected].move_status = 1;
				cc.prevx = x; cc.prevy = y;

			}
			cc.right_button_status = GLUT_DOWN;
		}
		else if (state == GLUT_UP) {
			if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
				move_mode = 1;
				cam[cam_selected].move_status = 0;
			}
			cc.right_button_status = GLUT_UP;
			shader_mode = PS;
			update_color();
		}
	}

	
	
}

void mouseWheel(int button, int dir, int x, int y)
{
	if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
		move_mode = 0;
	}
	if (move_mode == 0 || cam_selected == DYNAMIC_CCTV_VIEW) {
		move_mode = 1;
		if (dir < 0) {
			//	renew_cam_position(cam_selected, dir * 10, cam[cam_selected].naxis);
			if (cam[cam_selected].fov_y > 5 * TO_RADIAN) {
				cam[cam_selected].fov_y += dir * 0.01;
				ProjectionMatrix[cam_selected] = glm::perspective(cam[cam_selected].fov_y, cam[cam_selected].aspect_ratio, cam[cam_selected].near_clip, cam[cam_selected].far_clip);
				ViewProjectionMatrix[cam_selected] = ProjectionMatrix[cam_selected] * ViewMatrix[cam_selected];

				glutPostRedisplay();
			}
		}
		if (dir > 0) {
			if (cam[cam_selected].fov_y < 100 * TO_RADIAN) {
				cam[cam_selected].fov_y += dir * 0.01;
				ProjectionMatrix[cam_selected] = glm::perspective(cam[cam_selected].fov_y, cam[cam_selected].aspect_ratio, cam[cam_selected].near_clip, cam[cam_selected].far_clip);
				ViewProjectionMatrix[cam_selected] = ProjectionMatrix[cam_selected] * ViewMatrix[cam_selected];

				glutPostRedisplay();
			}
		}
	}
	else if(cam_selected != DYNAMIC_CCTV_VIEW) {
		if (dir < 0) {
			renew_cam_position(cam_selected, dir * 100, cam[cam_selected].naxis);
			set_ViewMatrix(cam_selected);
			ViewProjectionMatrix[cam_selected] = ProjectionMatrix[cam_selected] * ViewMatrix[cam_selected];


			glutPostRedisplay();

		}
		if (dir > 0) {
			renew_cam_position(cam_selected, dir * 100, cam[cam_selected].naxis);
			set_ViewMatrix(cam_selected);
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
	light_time = timestamp_scene % 10;
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
	int i;
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong.frag" },
	{ GL_NONE, NULL }
	};


	ShaderInfo shader_info_GS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");

	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_PS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_exponent");

	loc_wall_effect = glGetUniformLocation(h_ShaderProgram_PS, "wall_effect");
	loc_wall_width = glGetUniformLocation(h_ShaderProgram_PS, "wall_width");
	loc_blind_effect = glGetUniformLocation(h_ShaderProgram_PS, "u_blind_effect");
	loc_my_blind_effect = glGetUniformLocation(h_ShaderProgram_PS, "u_my_blind_effect");
	//loc_blind_effect = glGetUniformLocation(h_ShaderProgram_PS, "u_blind_effect");
	loc_blind_extent = glGetUniformLocation(h_ShaderProgram_PS, "u_blind_extent");
	loc_my_blind_extent = glGetUniformLocation(h_ShaderProgram_PS, "u_my_blind_extent");
	loc_cartoon_effect = glGetUniformLocation(h_ShaderProgram_PS, "u_cartoon_effect");
	loc_cartoon_levels = glGetUniformLocation(h_ShaderProgram_PS, "u_cartoon_levels");
	

	glUniform1i(loc_wall_effect, 0);
	glUniform1f(loc_wall_width, 0.1f);

	glUniform1i(loc_blind_effect, 0);
	glUniform1i(loc_my_blind_effect, 0);
	glUniform1i(loc_cartoon_effect, 0);

	glUniform1f(loc_cartoon_levels, 3.0f);

	h_ShaderProgram_GS = LoadShaders(shader_info_GS);
	loc_ModelViewProjectionMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrixInvTrans");
	/*
	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GS, string);
	}
	
	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_exponent");
	*/

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
	glDisable(GL_DEPTH_TEST); // Default state
	glEnable(GL_MULTISAMPLE);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.12f, 0.18f, 0.12f, 1.0f);
	initialize_camera();
	//
	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++)	light_power[i] = 1;
	prepare_rectangle();
	initialize_wall();
	initialize_blind();
	initialize_my_blind();
	initialize_cartoon();
	//
	initialize_lights_and_material();
}

void prepare_scene(void) {
	char car[][50] = { "Data/car_body_triangles_v.txt" ,"Data/car_wheel_triangles_v.txt","Data/car_nut_triangles_v.txt" };
	define_axes();
	define_static_objects();
	prepare_view();
	prepare_path();
	define_animated_tiger();

	prepare_path();
	prepare_geom_obj(GEOM_OBJ_ID_CAR_BODY, car[0], GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_WHEEL,car[1] , GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_NUT, car[2], GEOM_OBJ_TYPE_V);
	/////

	////
	prepare_cctv();
	for(int i=0; i<CAM_NUM ; i++)
		set_up_scene_lights(i);
	
	
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
