#include "../include/graphics/text.h"
#include "../include/logger.h"
#include "../include/glad/glad.h"
#include "freetype/freetype.h"
#include "../include/graphics/graphics.h"
#include "../include/graphics/camera.h"
#include "../include/files.h"

/**
* This function handles getting the codepoints given a utf-8 encoded string for functionality later in the program
*/
uint32_t* utf8_to_codepoints(const char *s, size_t *out_len) {
    size_t cap = 16;
    size_t len = 0;
    uint32_t *codepoints = malloc(cap * sizeof *codepoints);
    if (!codepoints) return NULL;

    const unsigned char *p = (const unsigned char*)s;
    while (*p) {
        uint32_t cp;
        size_t nbytes;

        if (*p < 0x80) {
            // 1‑byte sequence: 0xxxxxxx
            cp = *p;
            nbytes = 1;
        }
        else if ((p[0] & 0xE0) == 0xC0) {
            // 2‑byte sequence: 110xxxxx 10xxxxxx
            cp = p[0] & 0x1F;
            nbytes = 2;
        }
        else if ((p[0] & 0xF0) == 0xE0) {
            // 3‑byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
            cp = p[0] & 0x0F;
            nbytes = 3;
        }
        else if ((p[0] & 0xF8) == 0xF0) {
            // 4‑byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            cp = p[0] & 0x07;
            nbytes = 4;
        }
        else {
            // invalid leading byte
            free(codepoints);
            *out_len = 0;
            return NULL;
        }

        // Consume continuation bytes
        for (size_t i = 1; i < nbytes; ++i) {
            if ((p[i] & 0xC0) != 0x80) {
                // invalid continuation byte
                free(codepoints);
                *out_len = 0;
                return NULL;
            }
            cp = (cp << 6) | (p[i] & 0x3F);
        }

        // advance pointer
        p += nbytes;

        // append cp to array
        if (len + 1 > cap) {
            cap *= 2;
            uint32_t *tmp = realloc(codepoints, cap * sizeof *tmp);
            if (!tmp) {
                free(codepoints);
                *out_len = 0;
                return NULL;
            }
            codepoints = tmp;
        }
        codepoints[len++] = cp;
    }

    *out_len = len;
    return codepoints;
}

void init_font(font* font, struct shader* f_shader, char* font_otf, char* goal_source, int pixel_size, float scale_x, float scale_y){
    font->pixel_size = pixel_size;
    font->scale_x = scale_x;
    font->scale_y = scale_y;
    font->font_shader = f_shader;
    font->characters = malloc(sizeof(table)); // make the space in memory
    init_table(font->characters);

    // This sets up the buffers for the text rendering
    glGenVertexArrays(1, &font->t_vao);
    glGenBuffers(1, &font->t_vbo);
    glBindVertexArray(font->t_vao);
    glBindBuffer(GL_ARRAY_BUFFER, font->t_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    font_otf = get_font_path(font_otf); 
    log_debug("Making font %s", font_otf);

    FT_Library ft;
    if (FT_Init_FreeType(&ft)){
        log_error("ERROR::FREETYPE Could not init freetype library");
        return;
    }
    FT_Face face;
    if (FT_New_Face(ft, font_otf, 0, &face)){
        log_error("ERROR:FREETYPE Failed to load the font");
        return;
    }

    log_debug("SETTING PIXEL FACES FOR %s", font_otf);
    FT_Set_Pixel_Sizes(face, 0, font->pixel_size);

    log_debug("UNPACKING ALIGNMENT");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disalbe byte-alignment restriction

    log_debug("ADDING CHARS 0 - 128");
    for (uint32_t c = 0; c < 128; c++){
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)){
            log_error("FAILED TO LOAD GLYPH %c", c);
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        struct character ch;
        ch.TextureID = texture;
        vec2 temp = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
        vec2 temp2 = {face->glyph->bitmap_left, face->glyph->bitmap_top};

        glm_vec2_copy(temp, ch.size);
        glm_vec2_copy(temp2, ch.bearing);
        ch.advance = face->glyph->advance.x;

        table_set(font->characters, c, ch);
    }

    size_t n;
    uint32_t *codepoints = utf8_to_codepoints(goal_source, &n);
    for (size_t ind = 0; ind < n; ind++){
        uint32_t c = codepoints[ind];
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)){
            log_error("Failed to load glyph %u for font %s", c, font_otf);
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);


        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


        struct character character;
        character.TextureID = texture;
        vec2 temp = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
        vec2 temp2 = {face->glyph->bitmap_left, face->glyph->bitmap_top};


        glm_vec2_copy(temp, character.size);
        glm_vec2_copy(temp2, character.bearing);
        character.advance = face->glyph->advance.x;

        table_set(font->characters, c, character);

    }
    print_table(font->characters);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    log_debug("Done generating font for %s", font_otf);
}

void render_font(font* font, char* source, float x, float y, float scale, vec3 color, struct camera global_camera){
    font->font_shader->use(font->font_shader);
    // This assumes that the color in each text shader has the uniform name "text_color"
    glUniform3f(glGetUniformLocation(font->font_shader->ID, "text_color"), color[0], color[1], color[2]);
    font->font_shader->set_int(font->font_shader, "text", 5); 
    glActiveTexture(GL_TEXTURE5);
    glBindVertexArray(font->t_vao);

    mat4 model; glm_mat4_identity(model);
    font->font_shader->set_mat4(font->font_shader, "model", model);
    font->font_shader->set_mat4(font->font_shader, "projection", *global_camera.get_projection_matrix(&global_camera));
    font->font_shader->set_mat4(font->font_shader, "view", *global_camera.get_view_matrix(&global_camera));

    size_t length;
    uint32_t *codepoints = utf8_to_codepoints(source, &length);

    assert(length != 0); // dont let them render empty stuff and make suer nothing is broken

    for(size_t i = 0; i < length; i++){
        struct character ch;
        if (table_get(font->characters, codepoints[i], &ch)){
            float xpos = x + ch.bearing[0] * font->scale_x * scale;
            float ypos = y - (ch.size[1] - ch.bearing[1]) * font->scale_y * scale;

            float w = ch.size[0] * font->scale_x * scale;
            float h = ch.size[1] * font->scale_y * scale;

            // log_debug("CHARACTER %c X POS %f, Y POS %f", codepoints[i], xpos, ypos);
            // log_debug("                 W %f,     H %f", w, h);

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBindBuffer(GL_ARRAY_BUFFER, font->t_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.advance >> 6) * font->scale_x * scale;
        }
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
