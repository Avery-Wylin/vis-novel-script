#include "VNInterpreter.h"
#include "OperationDefs.h"
#include "VNAssetManager.h"

namespace VNOP{

    void load_ops_control() {
        operation_map["routine"] = routine;
        format_map[routine] = {"routine -file -label"};

        operation_map["jump"] =  jump;
        format_map[jump] = {"jump -label | -file"};

        format_map[jumpto] = {"jumpt -opnum"};

        format_map[ifbranch] = {"ifbranch -opnum -cond | -match"};

        operation_map["return"] =  jump_return;
        format_map[jump_return] = {"return"};

        operation_map["branch"] = branch;
        format_map[branch] = {"branch -label -var | -match"};

        operation_map["allow"] = allow;
        format_map[allow] = {"allow | -var -match"};

        operation_map["block"] =  block;
        format_map[block] = {"block | -var -match"};

        operation_map["match"] = match;
        format_map[match] = {"match -filter"};

        operation_map["option"] =  option;
        format_map[option] = {"option -case"};

        operation_map["lock-match"] = lock_match;
        format_map[lock_match] = {"lock-match"};

        operation_map["end-match"] = end_match;
        format_map[end_match] = {"end-match"};

        operation_map["exit"] = exit_interpreter;
        format_map[exit_interpreter] = {"exit"};

        operation_map["wait"] = wait;
        format_map[wait] = {"wait | -seconds -skippable"};

        operation_map["resume"] =  resume;
        format_map[resume] = {"resume"};

        operation_map["var"] =  def_var;
        format_map[def_var] = {"var -name -value"};

        operation_map["print"] =  print;
        format_map[print] = {"print | values..."};
    }

};

// routine <filename> <label>
void VNOP::routine( func_args ) {
    exact_args( 2 )
    VNInterpreter routine_vni;
    std::string file = args[0].value_string();
    std::string start = args[1].value_string();
    routine_vni.start_routine( file, start );
}


// jump -label
// jump -label -file
void VNOP::jump( func_args ) {
    if( args.size() == 1 ) {
        vni.jump( args[0].value_string() );
    }
    else if( args.size() == 2 ) {
        if( vni.switch_file( args[1].value_string(), true ) )
            vni.jump( args[0].value_string() );
        // If the file swap failed, skip the jump (jumps do not increment)
        else
            vni.skip();
    }
}

// NOTE this is not registered with the map, compiler generated
// jumpto -operation-number
void VNOP::jumpto(func_args){
    vni.jump( args[0].value_int(), false);
}

// return
void VNOP::jump_return( func_args ) {
    vni.jump_return();
}

// branch -label -bool
// branch -label -var -match
void VNOP::branch( func_args ) {
    if( args.size() == 2) {
        if(args[1].value_bool())
            vni.jump(args[0].value_string(), false);
        else
            vni.skip();
    }
    else if( args.size() == 3 ) {
        if(args[1].equals(args[2]))
            vni.jump(args[0].value_string(), false);
        else
            vni.skip();
    }
}

// NOTE this is not regeisterd with the map, compiler generated
// branchto -operation-number -val | -match
void VNOP::ifbranch(func_args){
    if(args.size() == 2){

        // skip if true
        if(args[1].value_bool())
            vni.skip();

        // jump to fail if false
        else
            vni.jump(args[0].value_int(), false);
    }
    else if(args.size()==3){

        // skip if matches
        if(args[1].equals(args[2]))
            vni.skip();

        // jump to fail if not
        else
            vni.jump(args[0].value_int(), false);
    }
}

// allow
// allow <bool>
// allow <match value> <var>
void VNOP::allow( func_args ) {
    if( args.size() == 0 )
        vni.filtered = false;
    else if( args.size() == 1 )
        vni.filtered |= !args[0].value_bool();
    else if( args.size() == 2 )
        vni.filtered |= !args[0].equals( args[1] );
}

// block
// block <bool>
// block <match value> <var>
void VNOP::block( func_args ) {
    if( args.size() == 0 )
        vni.filtered = true;
    else if( args.size() == 1 )
        vni.filtered |= args[0].value_bool();
    else if( args.size() == 2 )
        vni.filtered |= args[0].equals( args[1] );
}

// match -filter
void VNOP::match( func_args ) {
    ensure_args( 1 )
    // set a match filter
    vni.match_filter = args[0].value_string();
    vni.matching = true;
}

// option <var>
void VNOP::option( func_args ) {
    // Option should only be checked if matching
    if( !vni.matching )
        return;

    ensure_args( 1 )

    // if the variable matches, remove the filter
    if( vni.match_filter == args[0].value_string() ) {
        vni.match_filter.clear();
        vni.matching = false;
    }
}

// done
void VNOP::lock_match( func_args ) {
    // Done only sets unmatchable if not matching, this is done before calling
    // set the match filter to unmatchable (empty filter with true matching)
    vni.match_filter.clear();
    vni.matching = true;
}

// end-match
void VNOP::end_match( func_args ) {
    // remove matching filter
    vni.match_filter.clear();
    vni.matching = false;
}

// exit stops the interpreter, this also ends a routine
// exit
void VNOP::exit_interpreter( func_args ) {
    vni.exit();
}

// wait
// wait -time -skippable
void VNOP::wait( func_args ) {
    if( args.size() == 0 )
        VNI::wait();
    else if(args.size() == 1)
        VNI::wait( args[0].value_float() );
    else if(args.size() == 2)
        VNI::wait( args[0].value_float(), args[1].value_bool() );
}

// resume
void VNOP::resume( func_args ) {
    VNI::resume();
}

// Variable redefintion
// var <name> <value>
void VNOP::def_var( func_args ) {
    ensure_args( 2 )
    uint32_t vid = VNI::variables.get_id( args[0].value_string() );

    if( vid != 0 )
        args[1].copy( VNI::variables.at( vid ) );
}

void VNOP::print( func_args ) {
    // load_variables(op);

    VNVariable v;

    for( uint8_t i = 0; i < args.size(); ++i ) {
        printf( "%s ", args[i].value_string().c_str() );
    }

    printf( "\n" );
    fflush( stdout );
}
