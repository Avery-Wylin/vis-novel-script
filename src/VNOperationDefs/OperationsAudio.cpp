#include "VNInterpreter.h"
#include "OperationDefs.h"

namespace VNOP {
    void load_ops_audio() {
        // Plays a single sound one time, checks with audio sound library
        operation_map[ "sound"] = sound;
        format_map[sound] = {"sound -filename | -volume -pitch -position"};

        // Plays or unpauses the music track at the front of the music queue
        operation_map[ "music play"] = music_play;
        format_map[music_play] = {"music play | -sound -volume -pitch -position"};

        // Pauses the music
        operation_map[ "music pause"] = music_pause;
        format_map[music_pause] = {"music pause"};

        // Drops the track, use pause if you want to fade first
        operation_map[ "music stop"] = music_stop;
        format_map[music_stop] = {"music stop"};

        // Sets the music volume
        operation_map[ "music volume"] = music_volume;
        format_map[music_volume] = {"music volume -value"};

        // Sets the music pitch
        operation_map[ "music pitch"] = music_pitch;
        format_map[music_pitch] = {"music pitch -value"};
    }
};

// sound -filename | -volume -pitch -position
void VNOP::sound(func_args){
    if(args.size() == 1 )
        Audio::play_sound(args[0].value_string());
    if(args.size() == 2)
        Audio::play_sound(args[0].value_string(), args[1].value_float());
    if(args.size() == 3)
        Audio::play_sound(args[0].value_string(), args[1].value_float(), args[2].value_float());
    if(args.size() == 4){
        vec4 pos;
        args[3].value_vec(pos);
        Audio::play_sound_3D(args[0].value_string(), pos, args[1].value_float(), args[2].value_float());
    }
}

void VNOP::music_play(func_args){
    if(args.empty())
        Audio::play_music();
    else if(args.size() == 1)
        Audio::play_music(args[0].value_string());
    else if(args.size() == 2)
        Audio::play_music(args[0].value_string(), args[1].value_float(), args[2].value_float());
}

void VNOP::music_stop(func_args){
    Audio::stop_music();
}


void VNOP::music_pause(func_args){
    Audio::pause_music();
}

void VNOP::music_volume(func_args){
    Audio::music_source.set_volume(args[0].value_float());
}

void VNOP::music_pitch(func_args){
    Audio::music_source.set_pitch(args[0].value_float());
}
