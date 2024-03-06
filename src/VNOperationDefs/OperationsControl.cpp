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

//NOTE this is not registered with the map, compiler generated
// alias -name
void VNOP::alias(func_args){
    try{
        VNI::aliases[args[0].value_string()].run(vni);
    }
    catch(std::out_of_range &oor){
    }
}

// routine <filename> <label>
void VNOP::routine( func_args ) {
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
    try{
        uint32_t vid = VNI::variables.get_id( args[0].value_string() );
        args[1].copy( VNI::variables.at( vid ) );
    }
    catch(std::out_of_range &oor){
        // The variable should have already been defined, this is just in case
    }
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
