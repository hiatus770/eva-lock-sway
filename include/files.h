#pragma once 

char* get_path(char* dir); // This is just for anything in the project directory 

char* get_shader_path(char* dir); // This will automatically look in the right directory for the shaders (which is in src/shaders)

char* get_texture_path(char* dir); // This will automatically look for the texture in the right directory which is going to become (assets/textures/*)

char* get_font_path(char* dir); // This will automatically look for the texture in the right directory which is going to become (assets/textures/*)