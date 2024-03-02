#include "Mesh.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <iterator>

void Mesh::set_attribute_format( uint8_t attrb, uint8_t data_type, uint8_t vector_size) {
    // Return if invalid attribute or the attribute format is already set
    if( attrb >= NUM_ATTRBS || attributes[attrb].is_set || data_type == Attribute_Data::NONE )
        return;

    Attribute_Data &attribute = attributes[attrb];
    attribute.data_type = data_type;
    attribute.vector_size = vector_size;
    attribute.is_set = true;
    updated = true;
}

void Mesh::copy_attribute_format(const Mesh& src_mesh){
    for(uint8_t i = 0; i < NUM_ATTRBS; ++i){
        set_attribute_format(i, src_mesh.attributes[i].data_type, src_mesh.attributes[i].vector_size);
    }
}

// Append the indices in the range of the given partition. All indices are shifted by the vertex start
void Mesh::append_indices( std::vector<uint32_t> src_indices, const Partition &src_partition, uint32_t vertex_start ) {
    indices.reserve( indices.size() + src_partition.index_end - src_partition.index_begin );

    for( uint32_t i = src_partition.index_begin; i < src_partition.index_end; ++i ) {
        indices.push_back(src_indices[i] - src_partition.vertex_begin + vertex_start);
    }
}

// Check each set attribute and determine if it is append compatible
bool Mesh::is_append_compatible( const Mesh &other ) {
    for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
        if( attributes[i].is_set && ( !attributes[i].is_set || !attributes[i].is_append_compatible( other.attributes[i] ) ) ) {
            return false;
        }
    }

    return true;
}

void Mesh::add_partition( uint32_t v_count, uint32_t i_count ) {
    if( partitions.empty() ) {
        Partition n = {0, v_count, 0, i_count};
        partitions.push_back( n );
    }
    else {
        Partition &p = partitions.back();
        Partition n = { p.vertex_end,  p.vertex_end + v_count, p.index_end , p.index_end + i_count};
        partitions.push_back( n);
    }
}

// Blend an attribute of two meshes and set it to this mesh, the attribute will still match the size of this mesh
void Mesh::set_from_blend(const Mesh &src_mesh, const Mesh &other_mesh, uint8_t attrb, float f){
    if(src_mesh.partitions.back().vertex_end != other_mesh.partitions.back().vertex_end){
        puts("Unable to blend two meshes of non-matching size");
        fflush(stdout);
        return;
    }
    if( attrb > NUM_ATTRBS || !src_mesh.attributes[attrb].is_set || !other_mesh.attributes[attrb].is_set){
        puts("Unable to blend two meshes. Invalid attribute or attribute unset in either mesh.");
        fflush(stdout);
    }
    if( src_mesh.attributes[attrb].data_type != other_mesh.attributes[attrb].data_type ||
        src_mesh.attributes[attrb].vector_size != other_mesh.attributes[attrb].vector_size
    ){
        puts("Unable to blend two meshes with non-matching attribute type");
        fflush(stdout);
    }

    // Format the attribute if it is not yet set, do not clear the attribute, could be self-referenced
    set_attribute_format(attrb, src_mesh.attributes[attrb].data_type , src_mesh.attributes[attrb].vector_size );

    // Mix the attribute
    attributes[attrb].blend(src_mesh.attributes[attrb],other_mesh.attributes[attrb],f);

    updated = true;
}

// Append a partition of a mesh
void Mesh::append_mesh( const Mesh &src_mesh, uint32_t src_part_first, uint32_t src_part_last ) {
    // Copy the format if there are no partitions
    if(partitions.empty()){
        copy_attribute_format(src_mesh);
    }

    if( !is_append_compatible( src_mesh ) ) {
        puts( "Mesh can not be appended, is not append compatible." );
        fflush( stdout );
        return;
    }

    src_part_last = src_part_last < src_mesh.partitions.size()?src_part_last:src_mesh.partitions.size()-1;

    if( src_part_last >= src_mesh.partitions.size() || src_part_first > src_part_last ) {
        puts( "Mesh can not be appended, partition does not exist or first greater than last." );
        fflush( stdout );
        return;
    }

    for( uint32_t partition_id = src_part_first; partition_id <= src_part_last; ++partition_id ) {

        const Partition &p = src_mesh.partitions[partition_id];

        // Append the attributes
        for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
            if( attributes[i].is_set || partitions.empty() ) {
                attributes[i].append( src_mesh.attributes[i], p.vertex_begin, p.vertex_end );
            }
        }

        // Add a new partition
        add_partition( p.vertex_end - p.vertex_begin, p.index_end - p.index_begin );

        // Append the indices using the added partition's start
        append_indices( src_mesh.indices, p, partitions.back().vertex_begin );
    }
    updated = true;
}

void Mesh::append_mesh_transformed( Mesh &src_mesh,  mat4 transform, uint32_t src_part_first, uint32_t src_part_last) {
    // Copy the format if there are no partitions
    if(partitions.empty()){
        copy_attribute_format(src_mesh);
    }

    if( !is_append_compatible( src_mesh ) ) {
        puts( "Mesh can not be appended, is not append compatible." );
        fflush( stdout );
        return;
    }

    src_part_last = src_part_last < src_mesh.partitions.size()?src_part_last:src_mesh.partitions.size()-1;

    if( src_part_last >= src_mesh.partitions.size() || src_part_first > src_part_last ) {
        puts( "Mesh can not be appended, partition does not exist or first greater than last." );
        fflush( stdout );
        return;
    }


    for( uint32_t partition_id = src_part_first; partition_id <= src_part_last; ++partition_id ) {

        const Partition &p = src_mesh.partitions[partition_id];

        // Append the attributes
        for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
            if( attributes[i].is_set ) {
                switch( i ) {
                    case ATTRB_POS:
                        attributes[i].append_transformed( src_mesh.attributes[i], transform, false, p.vertex_begin, p.vertex_end );
                        break;

                    case ATTRB_NORM:
                        attributes[i].append_transformed( src_mesh.attributes[i], transform, true, p.vertex_begin, p.vertex_end );
                        break;

                    default:
                        attributes[i].append( src_mesh.attributes[i], p.vertex_begin, p.vertex_end );
                        break;
                }
            }
        }

        // Add a new partition
        add_partition( p.vertex_end - p.vertex_begin, p.index_end - p.index_begin );

        // Append the indices using the added partition's start
        append_indices( src_mesh.indices, p, partitions.back().vertex_begin );
    }
    updated = true;
}

void Mesh::remove_attribute( uint8_t attrb ) {
    attributes[attrb].clear();
    updated = true;
}

void Mesh::clear() {
    for(uint8_t i = 0; i < NUM_ATTRBS; ++i){
        attributes[i].clear();
    }
    indices.clear();
    partitions.clear();
    updated = true;
}

// Read a PLY file from assets and append it
void Mesh::append_PLY( std::string filename ) {

    // Open the assets model directory and look for the ply file
    std::string filepath = ( std::string )DIR_MODELS + filename + ".ply";
    std::ifstream filereader;
    filereader.open( filepath, std::ios::in | std::ios::binary );

    if( !filereader.is_open() ) {
        printf( "Unable to open file %s\n", filepath.c_str() );
        fflush( stdout );
        return;
    }

    // Vertex count, face count, and index
    uint32_t vertex_count = 0;
    uint32_t face_count = 0;

    // Store attribute vector sizes
    uint8_t attrb_vec_sizes[NUM_ATTRBS];
    uint8_t attrb_type[NUM_ATTRBS];

    for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
        attrb_type[i] = 0;
        attrb_vec_sizes[i] = 0;
    }


    // Link attribute names to the attribute, can detect invalid formats, order matters
    std::regex attrb_names[NUM_ATTRBS];
    attrb_names[0] = std::regex( "[xyz]" );                         // POS
    attrb_names[1] = std::regex( "n[xyz]" );                        // NORM
    attrb_names[2] = std::regex( "[st]" );                          // UV
    attrb_names[3] = std::regex( "(red)|(green)|(blue)|(alpha)" );  // COL
    attrb_names[4] = std::regex( "w[0-4]" );                        // WEIGHT
    attrb_names[5] = std::regex( "j[0-4]" );                        // JOINT
    attrb_names[6] = std::regex( "sk[xyz]" );                       // SK_POS
    attrb_names[7] = std::regex( "skn[xyz]" );                      // SK_NORM


    // Variables for parsing the header of a PLY file
    std::string property;
    std::vector<std::string> tokens;
    std::istringstream tokenstream;




    // Read each line until the "end_header" line

    // Save the last used attribute, if the order is broken, then exit
    uint8_t last_attribute = ATTRB_POS;

    while( property != "end_header" ) {
        std::getline( filereader, property, '\n' );
        tokenstream.str( property );
        tokens = std::vector<std::string>( std::istream_iterator<std::string>( tokenstream ), {} );

        // First token is 'property' and has 3+ tokens
        if( tokens.size() >= 3  && tokens[0] == "property" ) {

            // Determine the type
            uint8_t type = Attribute_Data::NONE;

            if( tokens[1] == "float" )
                type = Attribute_Data::FLOAT;
            else if( tokens[1] == "uchar" )
                type = Attribute_Data::BYTE;

            // Second token is 'float' or 'uchar'
            if( type != Attribute_Data::NONE ) {

                // Look at each attribute and find the matching name
                for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {

                    // Third token matches attribute name
                    if( std::regex_match( tokens[2], attrb_names[i] ) ) {

                        // Check for ascending order
                        if( last_attribute > i ) {
                            printf( "Attribute %s in file %s was declared out of order.\n", tokens[2].c_str(), filename.c_str() );
                            fflush( stdout );
                            return;
                        }

                        last_attribute = i;

                        // Exit if the property was redeclared with a new type
                        if( attrb_type[i] != Attribute_Data::NONE && attrb_type[i] != type ) {
                            printf( "Attribute %s was redeclared with a different type in file %s\n", tokens[2].c_str(), filename.c_str() );
                            fflush( stdout );
                            return;
                        }

                        // Set the attribute type and add to vector size
                        attrb_type[i] = type;
                        ++attrb_vec_sizes[i];

                        if( attrb_vec_sizes[i] > 4 ) {
                            printf( "Attribute %s has a vector size greater than 4 in file %s\n", tokens[2].c_str(), filename.c_str() );
                            fflush( stdout );
                            return;
                        }
                    }
                }
            }
        }

        // First token is "element", only 'vertex' and 'face' are used
        else if( tokens.size() >= 3 && tokens[0] == "element" ) {
            int count;

            try {
                count = std::stoi( tokens[2] );
            }
            catch( std::invalid_argument &ex ) {
                printf( "Element count was not an integer in %s\n", filepath.c_str() );
                fflush( stdout );
                return;
            }

            if( tokens[1] == "vertex" ) {
                vertex_count = count;
            }
            else if( tokens[1] == "face" ) {
                face_count = count;
            }
        }

        tokenstream.clear();
    }

    // Header has been processed, filereader should be on data segment
    std::istream::pos_type file_read_pos = filereader.tellg();
    filereader.clear();
    filereader.close();

    // If the partition table is empty, then set the attribute format
    if( partitions.empty() ) {
        clear();
        for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
            set_attribute_format( i, attrb_type[i], attrb_vec_sizes[i] );
        }
    }

    // Find the stop attribute (reduced iterations per vertex)
    uint8_t attrb_end = 0;

    // Check for conflicts with the header and the Mesh
    for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
        if( attrb_type[i] != Attribute_Data::NONE ) {
            attrb_end = attrb_end < i ? i : attrb_end;
        }

        if( attributes[i].is_set && (attrb_type[i] == Attribute_Data::NONE  || !attributes[i].is_append_compatible( attrb_type[i], attrb_vec_sizes[i] )) ) {
            printf( "Mesh is not append compatible with file %s\n", filename.c_str() );
            fflush( stdout );
            return;
        }

    }

    // Get the position where the file was reading and reopen in binary mode
    filereader.open( filepath, std::ios::binary );
    filereader.seekg( file_read_pos );

    // Create arrays now that sizes are known
    std::vector<char> attrb_data[NUM_ATTRBS];


    // Determine the sizes of each attribute's data
    for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
        if( attrb_vec_sizes[i] == 0 )
            continue;

        switch( attrb_type[i] ) {
            case Attribute_Data::NONE :
                continue;

            case Attribute_Data::FLOAT :
                attrb_data[i].reserve( vertex_count * attrb_vec_sizes[i] * 4 );
                continue;

            case Attribute_Data::BYTE :
                attrb_data[i].reserve( vertex_count * attrb_vec_sizes[i] );
                continue;
        }
    }

    // Data for triangulate faces
    std::vector<GLuint> faces;
    faces.reserve( face_count * 3 );
    char buffer[16];

    // Read each vertex and place the raw data into the vectors
    for( uint32_t i = 0; i < vertex_count; ++i ) {

        for( uint8_t i = 0; i <= attrb_end; ++i ) {
            switch( attrb_type[i] ) {
                case Attribute_Data::NONE :
                    continue;

                case Attribute_Data::FLOAT :
                    filereader.read( buffer, attrb_vec_sizes[i] * 4 );
                    attrb_data[i].insert( attrb_data[i].end(), buffer, buffer + attrb_vec_sizes[i] * 4 );
                    continue;

                case Attribute_Data::BYTE :
                    filereader.read( buffer, attrb_vec_sizes[i] );

                    // Convert RGB into sRGB
                    if( i == ATTRB_COL && attrb_vec_sizes[i] >= 3 ) {
                        for( uint8_t c = 0; c < 3; ++c ) {
                            buffer[c] = 255.0 * pow( ( ( uint8_t )buffer[c] ) / 255.0, 2.2 );
                        }
                    }

                    attrb_data[i].insert( attrb_data[i].end(), buffer, buffer + attrb_vec_sizes[i] );
                    continue;
            }
        }
    }

    // Read the face data section
    // Face data is stored as vertex count per face followed by the vertex indicies
    // For simplicity reasons, terminate if a non-triangle is found
    // Reuse the same buffer since it is an appropriate size for 4 uint32_t
    for( uint32_t i = 0; i < face_count; i++ ) {
        // Read the firt byte (1) + the last 3 uints (12) = 13 bytes
        filereader.read( buffer, 13 );

        // Only accept triangulate faces
        if( uint8_t( buffer[0] ) != 3 ) {
            printf( "Mesh not triangulate in file %s\n", filepath.c_str() );
            fflush( stdout );
            return;
        }

        // Read in the last 3 vertex integer pointers
        faces.insert( faces.end(), reinterpret_cast<uint32_t *>( buffer + 1 ), reinterpret_cast<uint32_t *>( buffer + 13 ) );
    }

    // Close file
    filereader.close();

    // Create a new partition
    add_partition(vertex_count, faces.size());

    // Append the indices
    append_indices( faces, {0, vertex_count, 0, (uint32_t)faces.size()}, partitions.back().vertex_begin);

    // Place the data into the attribute
    for( uint8_t i = 0; i <= attrb_end; ++i ) {
        if( attrb_type[i] == Attribute_Data::NONE )
            continue;

        attributes[i].append(attrb_data[i]);
    }
    updated = true;
}

// Load to VAO
void Mesh::to_VAO( VAO *vao, uint32_t partition_first, uint32_t partition_last ) {
    for( uint8_t i = 0; i < NUM_ATTRBS; ++i ) {
        if( attributes[i].is_set ) {

            // Convert color to float
            if( i == ATTRB_COL )
                attributes[i].load_vbo( i, vao, true );
            else
                attributes[i].load_vbo( i, vao );
        }
    }

    vao->load_index( indices.size(), indices.data() );
}

void Mesh::merge_partitions(){
    if(partitions.empty())
        return;
    uint32_t
    v_begin = partitions[0].vertex_begin,
    v_end = partitions.back().vertex_end,
    i_begin = partitions[0].index_begin,
    i_end = partitions.back().index_end;

    partitions.clear();
    partitions.push_back({v_begin, v_end, i_begin,i_end});
}

void Mesh::get_bounding_box( uint32_t partition, vec3* box){

    Attribute_Data &pos = attributes[ATTRB_POS];
    if(partitions.size() <= partition || !pos.is_set || pos.vector_size != 3)
        return;
    pos.get_min_max(box[0],box[1], partitions[partition].vertex_begin, partitions[partition].vertex_end);
    return;
}
