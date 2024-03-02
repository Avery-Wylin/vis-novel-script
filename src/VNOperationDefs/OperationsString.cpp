#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {

    void load_ops_string() {
        operation_map["str concat"] = str_concat;
        format_map[str_concat] = {"str concat -string -appended &dest"};

        operation_map["str clear"] = str_clear;
        format_map[str_clear] = {"str clear &dest"};

        operation_map["str length"] = str_length;
        format_map[str_length] = {"str length -string &length"};

        operation_map["str find"] = str_find;
        format_map[str_find] = {"str find -string -regex &begin &end"};

        operation_map["str erase"] = str_erase;
        format_map[str_erase] = {"str erase -string -begin -end &dest"};

        operation_map["str trim"] = str_trim;
        format_map[str_trim] = {"str trim -string -begin -end &dest"};

        operation_map["str extract"] = str_extract;
        format_map[str_extract] = {"str extract -string -regex &dest"};

        operation_map["str matches"] = str_matches;
        format_map[str_matches] = {"str matches -str -regex &bool"};

        operation_map["str replace"] = str_replace;
        format_map[str_replace] = {"str replace -string -regex -replacement &dest"};

        operation_map["str compose"] = str_compose;
        format_map[str_compose] = {"str compose &dest ..."};

    }
};

// str concat <str> <append str> <dest>
void VNOP::str_concat( func_args ) {
    exact_args( 3 )
    dest_last

    dest.cast( VAR_STRING );
    std::string s =  args[0].value_string() + args[1].value_string();
    dest.set( s );
}

// str clear <str>
void VNOP::str_clear( func_args ) {
    exact_args( 1 )
    dest_last
    dest.cast( VAR_STRING );
    dest.set( std::string("") );
}

// str length <str> <length>
void VNOP::str_length( func_args ) {
    exact_args( 2 )
    dest_last
    dest.set( ( int )args[0].value_string().size() );
}

// str find <str> <regex> <begin> <end>
void VNOP::str_find( func_args ) {
    exact_args( 4 )
    std::regex r;

    try {
        r = std::regex( args[1].value_string() );
    }
    catch( std::regex_error &regerror ) {
        VNDebug::runtime_error( "Invalid regex", args[1].value_string(), vni);
        return;
    }

    std::smatch matches;
    int begin = 0, end = 0;
    std::regex_search( args[0].value_string(), matches, r );

    if( !matches.empty() ) {
        begin = matches.position( 0 );
        end = matches.position( 0 ) + matches.length( 0 );
    }

    if( args[2].var_id != 0 )
        VNI::variables.at( args[2].var_id ).set( begin );

    if( args[3].var_id != 0 )
        VNI::variables.at( args[3].var_id ).set( end );

}

// str erase <str> <begin> <end> <dest>
void VNOP::str_erase( func_args ) {
    exact_args( 4 )
    dest_last
    string s = args[0].value_string();
    s.erase( s.begin() + args[1].value_int(), s.begin() + args[2].value_int() );
    dest.cast( VAR_STRING );
    dest.set( s );
}

// str trim <str> <begin> <end> <dest>
void VNOP::str_trim( func_args ) {
    exact_args( 4 )
    dest_last
    dest.cast( VAR_STRING );
    dest.set( args[0].value_string().substr( args[1].value_int(), args[2].value_int() ) );
}

// str extract <str> <regex> <dest>
void VNOP::str_extract( func_args ) {
    exact_args( 3 )
    dest_last

    dest.cast( VAR_STRING );

    std::regex r;

    try {
        r = std::regex( args[1].value_string() );
    }
    catch( std::regex_error &regerror ) {
        VNDebug::runtime_error( "Invalid regex", args[1].value_string(), vni);
        return;
    }

    std::string s = "";
    std::string v = args[0].value_string();
    std::string& cv = v;
    for (std::smatch matches; std::regex_search(cv, matches, r);)
    {
        s.append( matches.str() );
        cv = matches.suffix();
    }

    dest.set( s );
}

// str matches <str> <regex> <bool>
void VNOP::str_matches( func_args ) {
    exact_args( 3 )
    dest_last

    dest.cast( VAR_STRING );

    std::regex r;

    try {
        r = std::regex( args[1].value_string() );
    }
    catch( std::regex_error &regerror ) {
        VNDebug::runtime_error( "Invalid regex", args[1].value_string(), vni);
        return;
    }

    dest.set( std::regex_match( args[0].value_string(), r ) );
}

// str replace <str> <regex> <val> <dest>
void VNOP::str_replace( func_args ) {
    exact_args( 4 )
    dest_last

    dest.cast( VAR_STRING );

    std::regex r;

    try {
        r = std::regex( args[1].value_string() );
    }
    catch( std::regex_error &regerror ) {
        VNDebug::runtime_error( "Invalid regex", args[1].value_string(), vni);
        return;
    }

    dest.set( std::regex_replace( args[0].value_string(), r, args[2].value_string() ) );
}

// str compose <str> <vars...>
void VNOP::str_compose( func_args ) {
    ensure_args( 1 )

    if( args[0].var_id == 0 )
        return;

    VNVariable &dest = VNI::variables.at( args[0].var_id );
    dest.cast( VAR_STRING );
    std::string s;

    for( uint8_t i = 1; i < args.size(); ++i ) {
        s.append( args[i].value_string() );
    }

    dest.set( s );
}
