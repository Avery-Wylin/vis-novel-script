#ifndef TEXT_H
#define TEXT_H

#include <cglm/cglm.h>
#include "FontInfo.h"
#include <memory>
#include "../graphics/VAO.h"


class Text {
    private:
        string text;
        int32_t selStart = 0;
        int32_t selStop = 0;
        void generate(FontInfo &font);

        // Use a shared pointer to allow copy/deletion of text
        // copies of text will share the same VAO, the VAO will be deleted once no copies exist
        std::shared_ptr<VAO> vao;
        uint32_t vertexCount = 0;
        float width, height;
        float line_wrap = 0, ideal_ratio = 1;

    public:

        Text();
        virtual ~Text();

        uint32_t spacing = 20;
        bool needsGenerated = false;
        vec3 pos;
        float scale = 1;
        bool visible = true;

        void set_text(const std::string &text);
        void set_text(const char *text, uint32_t length);
        std::string const& get_text();
        void append(char c);
        void append(const std::string &s);
        void overwrite(char c);
        void insert(char c);
        void pop();
        void erase();
        void set_cursor(int32_t position);
        void move_cursor(int32_t amount);
        void select_more(int32_t amount);
        void select_all();
        void deselect();
        bool empty();
        void bind(FontInfo &font);
        inline void unbind(){vao->unbind();};
        inline uint32_t get_vertex_count(){return vertexCount;};
        inline float get_width(){return width;};
        inline float get_height(){return height;};
        inline uint32_t get_selection_text(){return selStop-selStart;};
        inline uint32_t length(){return text.length();}
        inline void set_ideal_ratio(float r){if(ideal_ratio != r){ideal_ratio = r; needsGenerated = true;}};


};

#endif /* TEXT_H */

