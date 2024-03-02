#ifndef MESH_H
#define MESH_H

#include "VAO.h"
#include "Shader.h"
#include <vector>
#include <string>
#include <cglm/mat4.h>
#include <algorithm>


class Attribute_Data {
        std::vector<float> data_float;
        std::vector<uint8_t> data_byte;

    public:
        static const uint8_t
        NONE = 0,
        FLOAT = 1,
        BYTE = 2;

        uint8_t data_type = NONE;
        uint8_t vector_size = 0;
        bool is_set = false;

        bool is_append_compatible( const Attribute_Data &other ) const {
            return
                data_type == other.data_type &&
                vector_size == other.vector_size;
        }

        bool is_append_compatible( uint8_t type, uint8_t v_size ) {
            return is_set && data_type == type  && vector_size == v_size;
        }

        uint32_t get_vertex_count() const {
            if( data_type == FLOAT ) {
                return data_float.size() / vector_size;
            }
            else if( data_type == BYTE ) {
                return data_byte.size() / vector_size;
            }

            return 0;
        }

        void append( std::vector<char> &src_data ) {
            if( data_type == Attribute_Data::FLOAT ) {
                data_float.reserve( src_data.size() / 4 );

                for( uint32_t i = 0; i < src_data.size(); i += 4 ) {
                    data_float.push_back( *reinterpret_cast<float *>( src_data.data() + i ) );
                }
            }
            else if( data_type == Attribute_Data::BYTE ) {
                data_byte.reserve( src_data.size() );

                for( uint32_t i = 0; i < src_data.size(); ++i ) {
                    data_byte.push_back( src_data[i] );
                }
            }
        }

        void blend(const Attribute_Data &src, const Attribute_Data &other, float f, uint32_t src_start = 0, uint32_t src_end = UINT32_MAX){
            // Ensure blending is possible, same as append compatible
            if(!is_append_compatible(src) || !src.is_append_compatible(other))
                return;

            // Ensure that the range is in bounds
            if( src_end > src.get_vertex_count() )
                src_end = src.get_vertex_count();
            if( src_start >= src_end)
                return;

            // The attribute will be written in-place, do not go out of bounds of what is already in this attribute
            // Vector size does not matter for lerping, do it component-wise
            if(data_type == FLOAT){
                for( uint32_t i = src_start*vector_size; i < src_end*vector_size && i < data_float.size(); ++i){
                    data_float[i] = src.data_float[i]+(other.data_float[i]-src.data_float[i])*f;
                }
            }
            else if(data_type == BYTE){
                for( uint32_t i = src_start*vector_size; i < src_end*vector_size && i < data_byte.size(); ++i) {
                    data_byte[i] = (uint8_t)(src.data_byte[i] + ( other.data_byte[i] - src.data_byte[i] ) * f);
                }
            }
        }


        void append( const Attribute_Data &src, uint32_t src_start = 0, uint32_t src_end = UINT32_MAX ) {
            if( !is_append_compatible( src ) )
                return;

            if( src_end > src.get_vertex_count() )
                src_end = src.get_vertex_count();

            if( src_start >= src_end )
                src_start = src_end;

            if( data_type == FLOAT ) {
                data_float.insert( data_float.end(), src.data_float.begin() + src_start * vector_size,  src.data_float.begin() + src_end * vector_size );
            }
            else if( data_type == BYTE ) {
                data_byte.insert( data_byte.end(), src.data_byte.begin() + src_start * vector_size,  src.data_byte.begin() + src_end * vector_size );
            }
        }

        void append_transformed( Attribute_Data &src, mat4 transform, bool is_normal, uint32_t src_start = 0, uint32_t src_end = UINT32_MAX ) {
            if( !is_append_compatible( src ) || src.vector_size != 3 || src.data_type != Attribute_Data::FLOAT ) {
                puts( "Could not append as a transformed attribute." );
                fflush( stdout );
                return;
            }

            if( src_end > src.get_vertex_count() )
                src_end = src.get_vertex_count();

            if( src_start >= src_end )
                return;

            data_float.reserve( ( src_end - src_start ) * 3 + data_float.size() );

            vec3 scale;
            mat4 normal_matrix;
            glm_mat4_inv( transform, normal_matrix );
            glm_mat4_transpose( normal_matrix );

            vec3 a =  GLM_VEC3_ZERO_INIT;

            for( uint32_t i = 0; i < src_end - src_start; ++i ) {
                a[0] = src.data_float[3 * ( i + src_start )];
                a[1] = src.data_float[3 * ( i + src_start ) + 1];
                a[2] = src.data_float[3 * ( i + src_start ) + 2];

                if( is_normal ) {
                    glm_mat4_mulv3( normal_matrix, a, 0, a );
                    glm_vec3_normalize( a );
                }
                else {
                    glm_mat4_mulv3( transform, a, 1, a );
                }

                data_float.push_back( a[0] );
                data_float.push_back( a[1] );
                data_float.push_back( a[2] );
            }
        }

        void get_min_max( vec3 min, vec3 max, uint32_t start = 0, uint32_t end = UINT32_MAX ) {
            glm_vec3_zero( min );
            glm_vec3_zero( max );

            if( vector_size != 3 || data_type != FLOAT || data_float.empty() )
                return;

            if( end > get_vertex_count() )
                end = get_vertex_count();

            if( start >= end )
                return;

            vec3 a;
            glm_vec3_copy( &data_float[0], min );
            glm_vec3_copy( min, max );

            for( uint32_t i = start; i < end; ++i ) {
                a[0] = data_float[3 * i];
                a[1] = data_float[3 * i + 1];
                a[2] = data_float[3 * i + 2];
                min[0] =  a[0] < min[0] ? a[0] : min[0];
                min[1] =  a[1] < min[1] ? a[1] : min[1];
                min[2] =  a[2] < min[2] ? a[2] : min[2];
                max[0] =  a[0] > max[0] ? a[0] : max[0];
                max[1] =  a[1] > max[1] ? a[1] : max[1];
                max[2] =  a[2] > max[2] ? a[2] : max[2];
            }
        }

        void load_vbo( uint8_t attribute, VAO *vao, bool byte_to_float = false ) {
            if( !is_set )
                return;

            if( data_type == FLOAT ) {
                vao->load_attrb_float( attribute, 0, 0, vector_size, data_float.size(), data_float.data() );
            }
            else if( data_type == BYTE ) {
                vao->load_attrb_byte( attribute, 0, 0, vector_size, data_byte.size(), byte_to_float, data_byte.data() );
            }
        }

        void clear() {
            data_float.clear();
            data_byte.clear();
            is_set = false;
        }
};

struct Partition {
    uint32_t vertex_begin, vertex_end, index_begin, index_end;
};

/*
 * Made of multiple partitions which designate the sizes of the mesh.
 * A partition is created on appending only.
 * A partition is removed on clearing only.
 * Appending can be done with either PLY files or another partition from another mesh.
 * Appending can be transformed
 * Attributes store the vbo data.
 * Indicies store the ibo data.
 * These are only changed on appending or clearing.
 * The attribute format is automatically set if it is not set on any append.
 */
class Mesh {

        std::vector<Partition> partitions;
        Attribute_Data attributes[NUM_ATTRBS];
        std::vector<uint32_t> indices;

        void append_indices( std::vector<uint32_t> src_indices, const Partition &src_partition, uint32_t vertex_start );
        bool is_append_compatible( const Mesh &other );
        void add_partition( uint32_t v_count, uint32_t i_count );

    public:
        bool updated = false;

        void set_attribute_format( uint8_t attrb, uint8_t data_type, uint8_t vector_size );
        void copy_attribute_format( const Mesh &src_mesh );
        void set_from_blend(const Mesh &src_mesh, const Mesh &other_mesh, uint8_t attribute, float f);
        void append_mesh( const Mesh &src_mesh, uint32_t src_part_first = 0, uint32_t src_part_last = UINT32_MAX );
        void append_mesh_transformed( Mesh &src_mesh, mat4 transform, uint32_t src_part_first = 0, uint32_t src_part_last = UINT32_MAX );
        void append_PLY( std::string filename );
        void remove_attribute( uint8_t attrb );
        void clear();
        void to_VAO( VAO *vao, uint32_t partition_first = 0, uint32_t partition_last = UINT32_MAX );
        void merge_partitions();
        void get_bounding_box( uint32_t partition, vec3 *box );
};

#endif // MESH_H
