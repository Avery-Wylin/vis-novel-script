#include "VAO.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <regex>

VAO::VAO() {
    vaoid = 0;
    for( int i = 0; i < Attribute::NUM_ATTRBS; i++ ) {
        vboids[i] = 0;
        vboSizes[i] = 0;
    }
}

VAO::~VAO() {
    free();
}

// Override the default behavior to forbid copy
VAO::VAO(VAO const&) {
    vaoid = 0;
    for( int i = 0; i < Attribute::NUM_ATTRBS; i++ ) {
        vboids[i] = 0;
        vboSizes[i] = 0;
    }
}

VAO& VAO::operator=(VAO const&){
    // Prevents = operator by setting VAO to itself
    std::cout << "Warning: VAO attempting operator=" << std::endl;
    return *this;
}

void VAO::allocate() {
    if( vaoid == 0 ) {
        glGenVertexArrays( 1, &vaoid );
    }
}

void VAO::free() {
    if( vaoid != 0 ) {
        glBindVertexArray( vaoid );
        for( int i = 0; i < Attribute::NUM_ATTRBS; i++ ) {
            if( vboids[i] != 0 ) {
                glDeleteBuffers( 1, &vboids[i] );
                vboids[i] = 0;
                vboSizes[i] = 0;
            }
        }
        glDeleteVertexArrays( 1, &vaoid );
        vaoid = 0;
    }
}

/*
 * Load an attribute buffer into the VAO.
 * If the attribute does not have a buffer, create one.
 * If the data being buffered is too large, reallocate memory, otherwise overwrite the memory.
 * A new buffer will not be created if offset is not 0.
 */
void VAO::load_attrb_float( int attrbid, int vertexOffset, int divisor, int vecSize, int size, void *data ) {
    // Invalid attribute id
    if( attrbid >= Attribute::NUM_ATTRBS || attrbid < 0 ) {
        fprintf( stderr, "Invalid attribute ID.\n" );
        return;
    }

    if( vecSize > 4 || vecSize < 1 ) {
        fprintf( stderr, "Attribute vector size must be 1 to 4.\n" );
        return;
    }

    // Attempt to create the VAO
    allocate();
    glBindVertexArray( vaoid );


    // Create the buffer if it has not been created
    if( vboids[attrbid] == 0 ) {
        glGenBuffers( 1, &vboids[attrbid] );
    }

    glBindBuffer( GL_ARRAY_BUFFER, vboids[attrbid] );

    // Compute the size of the offset in bytes
    int offset = vertexOffset * sizeof(GLfloat) * vecSize;

    // Compute data size
    int datasize = size * sizeof(GLfloat);

    // If there is an offset, always use a rewrite
    if( offset > 0 ) {
        glBufferSubData( GL_ARRAY_BUFFER, offset,  datasize<(vboSizes[attrbid] - offset)?datasize:(vboSizes[attrbid] - offset), data );
    }
    else {
        // Reallocate the buffer if it is too small
        if( vboSizes[attrbid] < datasize ) {
            glBufferData( GL_ARRAY_BUFFER, datasize, data, GL_DYNAMIC_DRAW );
            // Update the buffer size
            vboSizes[attrbid] = datasize;
        }
        else {
            glBufferSubData( GL_ARRAY_BUFFER, 0, datasize, data );
        }

    }

    // Set the attribute pointer
    glVertexAttribPointer( attrbid, vecSize, GL_FLOAT, GL_FALSE, 0, 0 );

    // Determine if an instance divisor is used
    glVertexAttribDivisor( attrbid, divisor );

    // Enable the attribute
    glEnableVertexAttribArray( attrbid );
}

void VAO::load_attrb_byte( int attrbid, int vertexOffset, int divisor, int vecSize, int size,  bool convert_float, void *data ) {

    // Invalid attribute id
    if( attrbid >= Attribute::NUM_ATTRBS || attrbid < 0 ) {
        fprintf( stderr, "Invalid attribute ID.\n" );
        return;
    }

    if( vecSize > 4 || vecSize < 1 ) {
        fprintf( stderr, "Attribute vector size must be 1 to 4.\n" );
        return;
    }

    // Attempt to create the VAO
    allocate();
    glBindVertexArray( vaoid );


    // Create the buffer if it has not been created
    if( vboids[attrbid] == 0 ) {
        glGenBuffers( 1, &vboids[attrbid] );
    }

    glBindBuffer( GL_ARRAY_BUFFER, vboids[attrbid] );

    // Compute the size of the offset in bytes
    int offset = vertexOffset * sizeof(GLbyte) * vecSize;

    // Compute data size
    int datasize = size * sizeof(GLbyte);

    // If there is an offset, always use a rewrite
    if( offset > 0 ) {
        glBufferSubData( GL_ARRAY_BUFFER, offset, datasize<(vboSizes[attrbid] - offset)?datasize:(vboSizes[attrbid] - offset), data );
    }
    else {
        // Reallocate the buffer if it is too small
        if( vboSizes[attrbid] < datasize ) {
            glBufferData( GL_ARRAY_BUFFER, datasize, data, GL_DYNAMIC_DRAW );
            // Update the buffer size
            vboSizes[attrbid] = datasize;
        }
        else {
            glBufferSubData( GL_ARRAY_BUFFER, 0, datasize, data );
        }

    }

    // Set the attribute pointer
    if(convert_float)
        glVertexAttribPointer( attrbid, vecSize, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0  );
    else
        glVertexAttribIPointer( attrbid, vecSize,  GL_UNSIGNED_BYTE, 0, 0 );

    // Determine if an instance divisor is used
    glVertexAttribDivisor( attrbid, divisor );

    // Enable the attribute
    glEnableVertexAttribArray( attrbid );
}


void VAO::load_index( uint32_t numIndices, GLuint *data ) {
    if( vaoid != 0 ) {
        glBindVertexArray( vaoid );
        if( iboid == 0 ) {
            glGenBuffers( 1, &iboid );
        }
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iboid );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( GLuint )*numIndices, data, GL_STATIC_DRAW );
        indexCount = numIndices;
    }
}

void VAO::bind() {
    if( vaoid != 0 ) {
        glBindVertexArray( vaoid );

        for( uint8_t i = 0; i < Attribute::NUM_ATTRBS; i++ ) {
            if( vboids[i] != 0 ) {
                glEnableVertexAttribArray( i );
            }
            else {
                glDisableVertexAttribArray( i );
            }
        }
    }
}

uint32_t VAO::get_index_count() {
    return indexCount;
}

void VAO::unbind() {
    glBindVertexArray( 0 );
}

void generateNormals( std::vector<float> &pos, std::vector<uint32_t> &index, std::vector<float> &norm ) {
    // This only works if the positions are separated, reused positions will be overwritten (okay in flat quads/polygons)

    vec3 normal, ab, ac;
    norm.insert(norm.begin(),pos.size(),0);
    std::vector<float> face_count;
    face_count.insert(face_count.begin(), pos.size()/3, 0);
    for(uint32_t i = 0; i < index.size(); i+=3){
        // cross ab and ac using the triangle positions
        glm_vec3_sub(&pos[index[i+1]*3], &pos[index[i]*3], ab);
        glm_vec3_sub(&pos[index[i+2]*3], &pos[index[i]*3], ac);
        glm_vec3_cross(ab,ac,normal);
        glm_vec3_normalize(normal);

        // add the normal
        glm_vec3_add(&norm[index[i]*3], normal  , &norm[index[i]*3]  );
        glm_vec3_add(&norm[index[i+1]*3], normal, &norm[index[i+1]*3]);
        glm_vec3_add(&norm[index[i+2]*3], normal, &norm[index[i+2]*3]);
        face_count[index[i]]++;
        face_count[index[i+1]]++;
        face_count[index[i+2]]++;
    }

    for(uint32_t i = 0; i < face_count.size(); ++i){
        norm[i*3] /= face_count[i];
        norm[i*3 + 1] /= face_count[i];
        norm[i*3 + 2] /= face_count[i];
    }
}

void VAO::load_ply( std::string filename ) {
    std::string filepath = (std::string)DIR_MODELS + filename + ".ply";
    std::ifstream filereader;
    filereader.open( filepath, std::ios::in | std::ios::binary );

    if( !filereader.is_open() ) {
        std::cout << "Unable to open file " << filepath << std::endl;
        return;
    }

    // Variables for counting the attributes
    uint32_t
    posCount = 0,
    normalCount = 0,
    uvcount = 0,
    colorCount = 0,
    vertexCount = 0,
    weightCount = 0,
    jointCount = 0,
    faceCount = 0;

    // Variables for parsing the header of a PLY file
    std::string property;
    std::vector<std::string> tokens;
    std::istringstream tokenstream;

    // Regex for determining if the names of a header match correctly
    std::regex
    posReg( "[xyz]" ),
            normalReg( "n[xyz]" ),
            uvReg( "[st]" ),
            colorReg( "(red)|(green)|(blue)|(alpha)" ),
            weightReg("w[0-4]"),
            jointReg("j[0-4]");



    // Read each line until the "end_header" line
    while( property != "end_header" ) {
        std::getline( filereader, property, '\n' );
        tokenstream.str( property );
        tokens = std::vector<std::string>( std::istream_iterator<std::string>( tokenstream ), {} );

        // Parse a property
        if( tokens.size() >= 2  && tokens[0] == "property" ) {
            // Position Property
            if( tokens[1] == "float" && std::regex_match( tokens[2], posReg ) ) {
                posCount++;
            }
            // Normal Attribute
            if( tokens[1] == "float" && std::regex_match( tokens[2], normalReg ) ) {
                normalCount++;
            }
            // UV Attribute
            if( tokens[1] == "float" && std::regex_match( tokens[2], uvReg ) ) {
                uvcount++;
            }
            // Color Attribute
            if( tokens[1] == "uchar" && std::regex_match( tokens[2], colorReg ) ) {
                colorCount++;
            }
            // Weight Attribute
            if( tokens[1] == "float" && std::regex_match( tokens[2], weightReg ) ) {
                weightCount++;
            }
            // Joint Attribute
            if( tokens[1] == "uchar" && std::regex_match( tokens[2], jointReg ) ) {
                jointCount++;
            }
        }
        // Parse an element, reads the number of elements
        else if( tokens.size() >= 3 && tokens[0] == "element" ) {
            int readInt;
            try {
                readInt = std::stoi( tokens[2] );
            }
            catch( std::invalid_argument &ex ) {
                std::cout << "Incorrect element count in " << filepath << std::endl;
                return;
            }
            if( tokens[1] == "vertex" ) {
                vertexCount = readInt;
            }
            else if( tokens[1] == "face" ) {
                faceCount = readInt;
            }
        }
        tokenstream.clear();
    }

    // Header has been processed, filereader should be on data segment
    // Get the position where the file was reading and reopen in binary mode
    std::istream::pos_type readPos = filereader.tellg();
    filereader.clear();
    filereader.close();
    filereader.open( filepath, std::ios::binary );
    filereader.seekg( readPos );

    // Create arrays now that sizes are known
    std::vector<GLuint> faces;
    faces.reserve( faceCount * 3 ); // Assume the faces are triangulated
    std::vector<float> pos;
    pos.reserve( vertexCount * posCount );
    std::vector<float> norm;
    norm.reserve( vertexCount * normalCount );
    std::vector<float> uv;
    uv.reserve( vertexCount * uvcount );
    std::vector<uint8_t> col;
    col.reserve( vertexCount * colorCount );
    std::vector<float> weight;
    weight.reserve( vertexCount * weightCount );
    std::vector<uint8_t> joint;
    joint.reserve( vertexCount * jointCount );

    // Read each attribute (in order) into the buffer then extract the correct amount
    char buffer[16];
    float *floatbuffer = reinterpret_cast<float *>( buffer );

    if(colorCount && !(colorCount == 3 || colorCount == 4)){
        printf("Color must be RGB or RGBA, %d channels were found for mesh %s.\n", colorCount, filename.data());
        return;
    }

    for( uint32_t i = 0; i < vertexCount; i++ ) {

        filereader.read( buffer, posCount * sizeof( float ) );
        pos.insert( pos.end(), floatbuffer, floatbuffer + posCount );

        filereader.read( buffer, normalCount * sizeof( float ) );
        norm.insert( norm.end(), floatbuffer, floatbuffer + normalCount );

        filereader.read( buffer, uvcount * sizeof( float ) );
        uv.insert( uv.end(), floatbuffer, floatbuffer + uvcount );

        filereader.read( buffer, colorCount );
        for(uint32_t c = 0; c < 3; ++c){
            // Convert colors from sRGB to Linear
            col.push_back( 255.0*pow(((uint8_t)buffer[c])/255.0, 2.2));
        }
        // Insert Alpha
        if(colorCount == 4){
            col.push_back(buffer[4]);
        }

        filereader.read( buffer, weightCount * sizeof( float ) );
        weight.insert( weight.end(), floatbuffer, floatbuffer + weightCount );

        filereader.read( buffer, jointCount );
        joint.insert( joint.end(), buffer, buffer + jointCount );

    }

    // Read the face data section
    // Face data is stored as vertex count per face followed by the vertex indicies
    // For simplicity reasons, terminate if a non-triangle is found
    // Reuse the same buffer since it is an appropriate size for 4 uint32_t
    for( uint32_t i = 0; i < faceCount; i++ ) {
        // Read the firt byte (1) + the last 3 uints (12) = 13 bytes
        filereader.read( buffer, 13 );

        // Only accept triangulate faces
        if( uint8_t( buffer[0] ) != 3 ) {
            std::cout << "Mesh not triangulate in file " << filepath << std::endl;
            std::cout << "A face of " << uint8_t( buffer[0] ) << " vertices was detected." << std::endl;
            return;
        }

        // Read in the last 3 vertex integer pointers
        faces.insert( faces.end(), reinterpret_cast<uint32_t *>( buffer + 1 ), reinterpret_cast<uint32_t *>( buffer + 13 ) );
    }

    // Close file
    filereader.close();

    // Load the VAO with the computed values
    if( posCount > 0 ) {
        this->load_attrb_float( Attribute::ATTRB_POS, 0, 0, posCount, pos.size(), pos.data() );
    }
    if( normalCount > 0 ) {
        this->load_attrb_float( Attribute::ATTRB_NORM, 0, 0, normalCount, norm.size(), norm.data() );
    }
    if( uvcount > 0 ) {
        this->load_attrb_float( Attribute::ATTRB_UV, 0, 0, uvcount, uv.size(), uv.data() );
    }
    if( colorCount > 0 ) {
        this->load_attrb_byte( Attribute::ATTRB_COL, 0, 0, colorCount, col.size(), true, col.data() );
    }
    if( weightCount > 0 ) {
        this->load_attrb_float( Attribute::ATTRB_WEIGHTS, 0, 0, weightCount, weight.size(), weight.data() );
    }
    if(jointCount > 0 ){
        this->load_attrb_byte(Attribute::ATTRB_JOINTS, 0, 0, jointCount, joint.size(), false, joint.data());
    }

    this->load_index( faces.size(), faces.data() );
}
