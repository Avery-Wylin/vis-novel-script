#ifndef AUDIO_H
#define AUDIO_H

#include "definitions.h"
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include "stb_vorbis.h"
#include "View.h"
#include <unordered_map>
#include <string>

struct SoundBuffer {
        ALuint buffer_id = 0;
        int channels = 0, sample_rate = 0;
        SoundBuffer() {}
        ~SoundBuffer() {
            free();
        }

        void load( std::string filename) {
            if( !buffer_id )
                alGenBuffers( 1, &buffer_id );

            short int *data = nullptr;
            std::string filepath = (std::string)DIR_SOUNDS + filename + ".ogg";
            int samples = stb_vorbis_decode_filename( filepath.c_str(), &channels, &sample_rate, &data );

            if( data == nullptr ) {
                printf( "Couldn't open audio file: %s\n", filename.c_str() );
                fflush( stdout );
            }

            alBufferData( buffer_id, channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, data, samples * channels * sizeof( short ), sample_rate );
            delete[] data;

        }

        void free() {
            if( !buffer_id )
                return;

            alDeleteBuffers( 1, &buffer_id );
            buffer_id = 0;
        }

    private:
        // Forbid copy
        SoundBuffer( SoundBuffer const & );
        SoundBuffer &operator=( SoundBuffer const & );
};


struct SoundSource {

        ALuint source_id = 0;

        SoundSource() {
        }

        ~SoundSource() {
            free();
        }

        void allocate() {
            alGenSources( 1, &source_id );
        }

        inline void set_pitch( float p ) {
            alSourcef( source_id, AL_PITCH, p );
        }

        inline void set_volume( float v ) {
            alSourcef( source_id, AL_GAIN, v );

        }

        inline void set_position( float x, float y, float z ) {
            alSource3f( source_id, AL_POSITION, x, y, z );
        }

        inline void set_relative( bool r ) {
            alSourcei( source_id, AL_SOURCE_RELATIVE, r );
        }

        inline void set_looping( bool l ) {
            alSourcei( source_id, AL_LOOPING, l );
        }

        inline void set_sound( SoundBuffer &s ) {
            alSourcei( source_id, AL_BUFFER, s.buffer_id );
        }

        inline void play() {
            alSourcePlay( source_id );
        }

        inline void pause() {
            alSourcePause( source_id );
        }

        inline void stop() {
            alSourceStop( source_id );
        }

        bool is_playing() {
            if( !source_id )
                return false;

            ALint state;
            alGetSourcei( source_id, AL_SOURCE_STATE, &state );
            return state == AL_PLAYING;
        }

        void free() {
            if( !source_id )
                return;

            stop();
            alDeleteSources( 1, &source_id );
            source_id = 0;
        }

    private:
        // Forbid copy
        SoundSource( SoundSource const & );
        SoundSource &operator=( SoundSource const & );

};


namespace Audio {

    extern ALCdevice *al_device;
    extern ALCcontext *al_context;

    // list of saved sounds
    extern std::unordered_map<std::string,SoundBuffer> sounds;

    // Music is generally looping and long, the sounds shouldn't be overriden
    extern SoundSource music_source;

    // Source pool for all sounds that are short and may be overriden
    extern SoundSource source_pool[AUDIO_POOL_SIZE];

    // Returns the first non-playing source, if none the length of the pool is returned
    unsigned int get_available_source();

    // Plays a relative sound, returns the source used, nullptr if it failed
    SoundSource *play_sound_3D( const std::string& sound,  vec3 pos, float volume = 1, float pitch = 1 );

    // Plays a non-relative sound, returns the source used, nullptr if it failed
    SoundSource *play_sound( const std::string& sound, float volume = 1, float pitch = 1 );

    // Special operations for the music buffer
    void play_music(const std::string & sound, float volume = 1, float pitch = 1);
    void play_music();
    void pause_music();
    void stop_music();

    // Initializes the OAL Soft context and devices, allocates the source pool
    void init();

    // Closes the AL device and free source pool
    void close();

    void set_master_volume( float v );
    void set_listener_transform( View &view );
};

#endif // AUDIO_H
