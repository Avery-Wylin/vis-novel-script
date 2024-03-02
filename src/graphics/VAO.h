#ifndef VAO_H
#define VAO_H

#include "library/glad_common.h"
#include "Shader.h"
#include <string>

class VAO {
public:
    
    GLuint vaoid = 0;
    GLuint vboids[Attribute::NUM_ATTRBS];
    GLuint iboid = 0;
    
    VAO();
    virtual ~VAO();

    static void unbind();

    void allocate();
    void free();
    void load_attrb_float(int attrbid, int vertexOffset, int divisor, int vecSize, int size, void* data);
    void load_attrb_byte(int attrbid, int vertexOffset, int divisor, int vecSize, int size, bool convert_float, void* data);
    void load_index(uint32_t numIndices, GLuint* data);
    void bind();
    uint32_t get_index_count();
    void load_ply(std::string filename);


private:
    VAO(VAO const&);
    VAO& operator=(VAO const&);
    uint32_t indexCount = 0;
    uint32_t vboSizes[Attribute::NUM_ATTRBS];
};

#endif /* VAO_H */

