#include "GUI.h"
#include <iostream>
#include <GLFW/glfw3.h>


/* GUI */


GUI::GUI() {
}

GUI::~GUI() {
    // Delete unremoved text pointers
    for( Element &e : elements ) {
        if( e.text != nullptr ) {
            delete e.text;
        }
    }
}

bool GUI::containsPoint( Element &e, float x, float y, float ratio ) {
    // Determine if the position is included in the element
    float x2 = e.w / ratio;
    // Rectangle inclusion, TODO make the bevel work as well
    return( x > e.x && x < ( e.x + x2 ) && y > e.y && y < ( e.y + e.h ) );
}

// Converts the given gui screen coordinate to a localized element coordinate
void GUI::toElementCoords( Element &e, float x, float y, float ratio, float &dx, float &dy ){
    dx = (x - e.x )/(e.w /ratio);
    dy = (y - e.y )/e.h;
}

uint8_t GUI::cursorSelect( float x, float y, float ratio ) {

    //Check if the current selected element is still valid
    float dx,dy;
    if( selectedElement != null && containsPoint( elements[selectedElement], x, y, ratio ) ) {
        toElementCoords(elements[selectedElement],x,y,ratio,dx,dy);
        onReselect( selectedElement , dx, dy);
        return selectedElement;
    }

    // Iterate through all element and return the first to include the cursor
    for( int i = 0; i < elements.size(); i++ ) {
        Element &e = elements[i];

        // Exclude hidden or unallocated elements
        if( !( e.flags & Element::ALLOCATED ) || !( e.flags & Element::VISIBLE ) )
            continue;
        if( containsPoint( e, x, y, ratio ) ) {
            onDeselect( selectedElement );
            selectedElement = i;
            toElementCoords(elements[selectedElement],x,y,ratio,dx,dy);
            onSelect( selectedElement, dx, dy );
            return i;
        }
    }
    onDeselect( selectedElement );
    selectedElement = null;
    return null;
}

void GUI::onSelect( uint8_t id, float x, float y ) {
    if( id == null )
        return;
    Element &e = elements[id];
    switch(e.type){

        case TEXT:
            selectedElement = null;
            break;

        case KEY_CAPTURE:
        case TOGGLE:
            e.flags ^= Element::ACTIVE;
            break;

        case TEXT_INPUT:
            e.text->setCursor(e.text->getText().size());
            e.flags |= Element::ACTIVE;
            break;

        case SLIDE:
            e.flags |= Element::ACTIVE;
            // e.value_float = x;
            // char num[5];
            // snprintf(num,5,"%.2f",e.value_float );
            // e.text->setText( num, 0 );
            break;

        case BUTTON:
            e.flags |= Element::ACTIVE;
            break;
    }

}

void GUI::onReselect( uint8_t id, float x, float y ) {
    if( id == null )
        return;
    Element &e = elements[id];

    switch(e.type){
        case TEXT:
        case BUTTON:
            break;

        case TEXT_INPUT:
            e.text->selectAll();
            break;

        case SLIDE:
            e.value_float = x * e.value_int;
            char num[8];
            snprintf(num,8,"%.2f",e.value_float );
            e.text->set_text( num, 0 );
            break;

        case KEY_CAPTURE:
        case TOGGLE:
            e.flags ^= Element::ACTIVE;
            break;
    }
}

void GUI::onDeselect( uint8_t id ) {
    if( id == null )
        return;
    Element &e = elements[id];
    switch(e.type){
        case TEXT_INPUT:
            e.text->deselectAll();

        case KEY_CAPTURE:
        case TEXT:
        case SLIDE:
            e.flags &= ~Element::ACTIVE;
            break;

        case TOGGLE:
        case BUTTON:
            break;
    }

}

void GUI::drawBackground( Shader &shader, float ratio ) {
    // The GUIManager will bind the Shader VAO and Camera
    for( Element &e : elements ) {
        if( !( e.flags & Element::ALLOCATED ) || !( e.flags & Element::VISIBLE ) )
            continue;
        shader.uniformVec4f( Uniform::UNIFORM_TRANSFORM, vec4{e.x * ratio, e.y, e.w, e.h} );
        // Size of edge bevel, larger number, smaller bevel, must exceed or equal 1
        shader.uniformFloat( Uniform::UNIFORM_FACTOR, fmax(40.0f, 1.0f/fmin(e.w,e.h )) );
        shader.uniformVec3f( Uniform::UNIFORM_COLOR, e.flags & Element::ACTIVE ? colorBackSelect : colorBack );
        shader.uniformVec3f( Uniform::UNIFORM_COLOR2, e.flags & Element::ACTIVE ? colorTextSelect : colorTextHighlight );
        glDrawArrays( GL_QUADS, 0, 4 );

        if(e.type == SLIDE){
            shader.uniformVec4f( Uniform::UNIFORM_TRANSFORM, vec4{e.x * ratio, e.y, e.w *e.value_float/e.value_int, e.h } );
            // shader.uniformFloat( Uniform::UNIFORM_FACTOR, 1.0f/fmin(e.w *e.value_float/e.value_int, e.h) );
            shader.uniformFloat( Uniform::UNIFORM_FACTOR, fmax(40.0f, 1.0f/fmin(e.w,e.h )) );
            shader.uniformVec3f( Uniform::UNIFORM_COLOR,  e.flags & Element::ACTIVE ? colorText : colorTextHighlight );
            glDrawArrays( GL_QUADS, 0, 4 );
        }
    }
}

void GUI::drawDial( Shader &shader, float ratio ) {
    // TODO ideally only draw if the selected type is a dial
}

void GUI::drawTexts( Shader &shader, FontInfo &font, float ratio ) {
    float scale;
    shader.uniformVec3f( Uniform::UNIFORM_COLOR2, colorTextHighlight );
    for( Element &e : elements ) {
        if( !( e.flags & Element::ALLOCATED ) || !( e.flags & Element::VISIBLE ) || e.text == nullptr )
            continue;
        e.text->bind( font );
        scale = fmin( e.w / e.text->getWidth(),  e.h / e.text->getHeight() );
        shader.uniformVec3f( Uniform::UNIFORM_TRANSFORM, vec3{e.x * ratio + (e.flags&Element::LEFT_ALIGN?0:0.5f) * ( e.w - e.text->getWidth()*scale ), e.y + 0.5f * ( e.h + scale * e.text->getHeight() ), scale} );
        shader.uniformVec3f( Uniform::UNIFORM_COLOR,  e.flags & Element::ACTIVE ? colorTextSelect : colorText );
        glDrawArrays( GL_QUADS, 0, e.text->getVertexCount() );
        e.text->unbind();
    }
}



uint8_t GUI::nextEmptyElement() {
    for( int i = 0; i < elements.size(); i++ ) {
        if( !( elements[i].flags & Element::ALLOCATED ) )
            return i;
    }
    // Forbid going past number of ids
    if( elements.size() >= null )
        return null - 1;
    // Create new element
    elements.push_back( Element() );
    return elements.size() - 1;
}

/* GUI MANAGER */

GUIManager::GUIManager() {
}

GUIManager::~GUIManager() {
}

void GUIManager::init() {
    textShader.load( "text2D" );
    backgroundShader.load( "background" );
    float pos[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    float uv[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    quadVAO.loadAttributeFloat( Attribute::ATTRB_POS, 0, 0, 2, 8, static_cast<void *>( pos ) );
    quadVAO.loadAttributeFloat( Attribute::ATTRB_UV, 0, 0, 2, 8, static_cast<void *>( uv ) );
    font.load( "liberation-mono" );
    glm_ortho( 0, ratio, 0, 1, 0, 1, orthoMat );
}

void GUIManager::selectGUI( uint8_t guiid ) {
    if( guiid < guis.size() && !guis[guiid].isRemoved() )
        selected = guiid;
    else
        selected = null;
}

void GUIManager::deselectGUI() {
    selected = null;
}


uint8_t GUIManager::create() {
    // Replace the old removed gui with a new one
    for( int i = 0; i < guis.size(); i++ ) {
        if( guis[i].isRemoved() ) {
            guis[i] = GUI();
            return i;
        }
    }
    // Create a new position and return
    if( guis.size() < null ) {
        guis.push_back( GUI() );
        return guis.size() - 1;
    }
    return null;
}

void GUIManager::remove( uint8_t &guiid ) {
    if( guiid < guis.size() )
        guis[guiid].markRemoved();
    if( selected == guiid )
        selected = null;
}

void GUIManager::draw() {
    if( selected == null )
        return;

    quadVAO.bind();
    Shader::bind(backgroundShader);
    Shader::uniformMat4f( UNIFORM_CAMERA, orthoMat );
    guis[selected].drawBackground( backgroundShader, ratio );

    Shader::bind(textShader);
    Shader::uniformMat4f( UNIFORM_CAMERA, orthoMat );
    font.fontTexture.bind( 0 );
    guis[selected].drawTexts( textShader, font, ratio );
    Shader::unbind();
}


uint8_t GUIManager::cursorSelect( float x, float y ) {
    if( selected == null )
        return null;
    return guis[selected].cursorSelect( x, y, ratio );
}

void GUIManager::setColor( const vec3 &a, const vec3 &b ) {
    if( selected == null )
        return;
    GUI &gui = guis[selected];


    // Set the background to the primary color
    gui.colorBack[0] = a[0];
    gui.colorBack[1] = a[1];
    gui.colorBack[2] = a[2];

    // Set the selected background to a mix of primary and secondary
    gui.colorBackSelect[0] = 0.5 * ( a[0] + b[0] );
    gui.colorBackSelect[1] = 0.5 * ( a[1] + b[1] );
    gui.colorBackSelect[2] = 0.5 * ( a[2] + b[2] );

    // Set unselected text to black/white tinted with secondary
    if( gui.colorBack[0] + gui.colorBack[1] + gui.colorBack[2] > 1.5 ) {
        gui.colorText[0] = 0.5 * b[0];
        gui.colorText[1] = 0.5 * b[1];
        gui.colorText[2] = 0.5 * b[2];
    }
    else {
        gui.colorText[0] = 0.5 * ( 1 + b[0] );
        gui.colorText[1] = 0.5 * ( 1 + b[1] );
        gui.colorText[2] = 0.5 * ( 1 + b[2] );
    }

    // Set selected text to black/white based on background
    if( gui.colorBackSelect[0] + gui.colorBackSelect[1] + gui.colorBackSelect[2] > 1.5 ) {
        glm_vec3_broadcast( 0, gui.colorTextSelect );
    }
    else {
        glm_vec3_broadcast( 1, gui.colorTextSelect );
    }

    // Set higlighted text to secondary
    gui.colorTextHighlight[0] = b[0];
    gui.colorTextHighlight[1] = b[1];
    gui.colorTextHighlight[2] = b[2];

}

// Exposed GUI functions
void GUI::removeElement( uint8_t id ) {
    if( !elementExist( id ) )
        return;

    // Delete text pointer if present
    if( elements[id].text != nullptr )
        delete elements[id].text;

    // Clear by setting the element to a new construction
    elements[id] = Element();
}

void GUI::setElementText( uint8_t id,  std::string text ) {
    if( !elementExist( id ) || elements[id].text == nullptr )
        return;
    elements[id].text->setText( text );
}

bool GUI::elementExist( uint8_t id ) {
    return id < elements.size() && ( elements[id].flags & Element::ALLOCATED );
}

uint8_t GUIManager::addText( const vec4 &bounds, const char *text ) {
    if(selected == null)
        return null;
    GUI &gui = guis[selected];

    uint8_t id =  gui.nextEmptyElement();
    if( id == null )
        return null;
    Element &e = gui.elements[id];
    e.flags = Element::ALLOCATED | Element::VISIBLE;
    e.type = TEXT;
    e.x = bounds[0];
    e.y = bounds[1];
    e.w = bounds[2];
    e.h = bounds[3];
    e.text = new Text();
    e.text->setText( text, 0 );
    return id;
}

uint8_t GUIManager::addInputText( const vec4 &bounds, const char *text, uint8_t charlimit ) {
    if(selected == null)
        return null;
    GUI &gui = guis[selected];

    uint8_t id =  gui.nextEmptyElement();
    if( id == null )
        return null;
    Element &e = gui.elements[id];
    e.flags = Element::ALLOCATED | Element::VISIBLE | Element::LEFT_ALIGN;
    e.type = INPUT_TEXT;
    e.x = bounds[0];
    e.y = bounds[1];
    e.w = bounds[2];
    e.h = bounds[3];
    e.text = new Text();
    e.text->setText( text, 0 );
    e.value_int = charlimit;
    return id;
}

uint8_t GUIManager::addButton( const vec4 &bounds, const char *label) {
    if(selected == null)
        return null;
    GUI &gui = guis[selected];

    uint8_t id =  gui.nextEmptyElement();
    if( id == null )
        return null;
    Element &e = gui.elements[id];
    e.flags = Element::ALLOCATED | Element::VISIBLE;
    e.type = BUTTON;
    e.x = bounds[0];
    e.y = bounds[1];
    e.w = bounds[2];
    e.h = bounds[3];
    e.text = new Text();
    e.text->setText( label, 0 );
    return id;
}

uint8_t GUIManager::addToggle( const vec4 &bounds, const char *label) {
    if( selected == null )
        return null;
    GUI &gui = guis[selected];

    uint8_t id =  gui.nextEmptyElement();
    if( id == null )
        return null;
    Element &e = gui.elements[id];
    e.flags = Element::ALLOCATED | Element::VISIBLE;
    e.type = TOGGLE;
    e.x = bounds[0];
    e.y = bounds[1];
    e.w = bounds[2];
    e.h = bounds[3];
    e.text = new Text();
    e.text->setText( label, 0 );
    return id;
}


uint8_t GUIManager::addSlide( const vec4 &bounds, uint32_t multiplier){
    if(selected == null)
        return null;
    GUI &gui = guis[selected];

    uint8_t id = gui.nextEmptyElement();
    if(id == null)
        return null;
    Element &e = gui.elements[id];
    e.flags = Element::ALLOCATED | Element::VISIBLE;
    e.type = SLIDE;
    e.x = bounds[0];
    e.y = bounds[1];
    e.w = bounds[2];
    e.h = bounds[3];
    e.text = new Text();
    e.text->setText( "0", 0 );
    e.value_int = multiplier;
    return id;
}

uint8_t GUIManager::addKeyCapture( const vec4 &bounds){
    if(selected == null)
        return null;
    GUI &gui = guis[selected];

    uint8_t id = gui.nextEmptyElement();
    if(id == null)
        return null;
    Element &e = gui.elements[id];
    e.flags = Element::ALLOCATED | Element::VISIBLE;
    e.type = KEY_CAPTURE;
    e.x = bounds[0];
    e.y = bounds[1];
    e.w = bounds[2];
    e.h = bounds[3];
    e.text = new Text();
    e.text->setText( "", 0 );
    return id;
}

float GUIManager::getFloatValue(uint8_t guiid, uint8_t element){
    if(guiid > guis.size())
        return 0;
    if(guis[guiid].elementExist(element)){
        return guis[guiid].elements[element].value_float;
    }
    return 0;
}

uint32_t GUIManager::getIntValue(uint8_t guiid, uint8_t element){
    if(guiid > guis.size())
        return 0;
    if(guis[guiid].elementExist(element)){
        return guis[guiid].elements[element].value_int;
    }
    return 0;
}

void GUIManager::setText(uint8_t guiid, uint8_t element, string &text){
    if(guiid > guis.size())
        return;
    if(guis[guiid].elementExist(element)){
        guis[guiid].elements[element].text->setText(text);
    }
}

void GUIManager::charInsert( char c ) {
    if(selected == null)
        return;
    GUI &gui = guis[selected];

    if( gui.selectedElement == null )
        return;
    Element &e = gui.elements[gui.selectedElement];
    if( e.type == ElementType::INPUT_TEXT && ( e.value_int > e.text->getText().size() || e.text->getSelectionCount()>0)) {
        e.text->overwrite(c);
    }
}

void GUIManager::charDelete() {
     if(selected == null)
        return;
    GUI &gui = guis[selected];

    if( gui.selectedElement == null )
        return;
    Element &e = gui.elements[gui.selectedElement];
    if( e.type == ElementType::INPUT_TEXT ) {
        e.text->erase();
    }
}

void GUIManager::keyPress(int32_t key, uint8_t modifiers){
    if(selected == null)
        return;
    GUI &gui = guis[selected];
    if( gui.selectedElement == null )
        return;
    Element &e = gui.elements[gui.selectedElement];

    if(e.type == KEY_CAPTURE && key > 0 && key < 340){
        e.value_int = key;
        e.modifiers = modifiers;

        std::string lable;

        // Append key if it is ascii
        if(key < 255)
            lable.push_back(key);
        else
            lable.append(std::to_string(key));

        if(modifiers & GLFW_MOD_CONTROL)
            lable.append("+ctrl");
        if(modifiers & GLFW_MOD_ALT)
            lable.append("+alt");
        if(modifiers & GLFW_MOD_SHIFT)
            lable.append("+shift");

        e.text->setText(lable.c_str(),0);
        gui.onDeselect(gui.selectedElement);
        gui.selectedElement = null;
    }
}

void GUIManager::inputArrowShift(int32_t amount){
    if(selected == null)
        return;
    GUI &gui = guis[selected];

    if( gui.selectedElement == null )
        return;
    Element &e = gui.elements[gui.selectedElement];
    if( e.type == ElementType::INPUT_TEXT ) {
        e.text->selectMore(amount);
    }
    else if(e.type == ElementType::SLIDE){
        e.value_float = fmin(fmax(e.value_float + amount / 100.0f * e.value_int,0),e.value_int);
        char num[8];
        snprintf(num,8,"%.2f",e.value_float );
        e.text->setText( num, 0 );
    }
}

void GUIManager::inputArrow(int32_t amount){
     if(selected == null)
        return;
    GUI &gui = guis[selected];
    if( gui.selectedElement == null )
        return;
    Element &e = gui.elements[gui.selectedElement];
    if( e.type == ElementType::INPUT_TEXT ) {
        e.text->moveCursor(amount);
    }
    else if(e.type == ElementType::SLIDE){
        e.value_float = fmin(fmax(e.value_float + amount /10.0f * e.value_int,0),e.value_int);;
        char num[8];
        snprintf(num,8,"%.2f",e.value_float );
        e.text->setText( num, 0 );
    }
}
