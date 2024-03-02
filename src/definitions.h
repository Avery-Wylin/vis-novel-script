#ifndef CFG_VARS_H
#define CFG_VARS_H

#include <inttypes.h>

// Audio
#define AUDIO_POOL_SIZE 16

// Armature
#define ARMATURE_MAX_JOINTS 150 // Must match shader uniform
#define ARMATURE_CONSTRAINT_TIMESTEP .05f

// Window
#define FPS 30

// FBO
#define FBO_MAX_COLOR_ATTACHMENTS 8

// View
#define VIEW_NEAR .1f
#define VIEW_FAR 100.0f

// Variables
#define MAX_ARGS 250


// File Directories
#define DIR_CONFIGS   "../config/"
#define DIR_SHADERS   "../assets/shaders/"
#define DIR_SAVES     "../saves/"
#define DIR_TEXTURES  "../assets/textures/"
#define DIR_MODELS    "../assets/models/"
#define DIR_ARMATURES "../assets/armatures/"
#define DIR_SOUNDS    "../assets/sounds/"
#define DIR_FONTS     "../assets/fonts/"
#define DIR_SCRIPTS   "../scripts/"

#endif // CFG_VARS_H
