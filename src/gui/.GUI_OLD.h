#ifndef GUI_H
#define GUI_H

#include <inttypes.h>
#include <vector>
#include <cglm/cglm.h>
#include "gui/FontInfo.h"
#include "gui/Text.h"
#include "graphics/VAO.h"

#define null (255)

enum ElementType : uint8_t {
    TEXT,       // Simple text container, no interaction
    BUTTON,     // Can be enabled, but code must disable it
    TOGGLE,     // Can be enabled/diabled by clicking again
    INPUT_TEXT, // Text that can be selected and edited
    SLIDE,      // Select a float value from 0 to 1 based on x position
    KEY_CAPTURE // Capture whichever key is pressed
};


struct Element {
    static const uint8_t ALLOCATED = 1, VISIBLE = 2, ACTIVE = 4, LEFT_ALIGN = 8;
    Text *text = nullptr;
    float x, y, w, h;
    float value_float = 0; // Value has different uses
    uint32_t value_int = 0;
    ElementType type = TEXT;
    uint8_t flags = 0;
    uint8_t modifiers = 0;
};

class GUI {
    public:l
        bool removed = false;
        uint8_t selectedElement = null;
        std::vector<Element> elements;
        // Bool to determine if it captures cursor input
        // Bool to determine if it captures nav keys

        // Exposed variables
        vec3 colorBackSelect = {1, 1, 1};
        vec3 colorTextSelect = {0, 0, 0};
        vec3 colorBack = {0, 0, 0};
        vec3 colorText = {1, 1, 1};
        vec3 colorTextHighlight = {0, 0.5, 1};

        GUI();
        virtual ~GUI();

        // Functions are not exposed to the user
        uint8_t cursorSelect( float x, float y, float ratio );
        void drawBackground( Shader &shader, float ratio );
        void drawDial( Shader &shader, float ratio );
        void drawTexts( Shader &shader, FontInfo &font, float ratio );
        bool containsPoint( Element &e, float x, float y, float ratio ); // Returns whether an element contains a given point
        void toElementCoords( Element &e, float x, float y, float ratio, float &dx, float &dy ); // Returns whether an element contains a given point
        void onSelect( uint8_t element, float x, float y );
        void onDeselect( uint8_t element );
        void onReselect( uint8_t element, float x, float y );
        uint8_t nextEmptyElement();
        uint8_t nextEmptyText();
        void removeElement( uint8_t id );
        void setElementText( uint8_t id,  std::string text );
        bool elementExist( uint8_t id );
        inline bool isRemoved() {
            return removed;
        };
        inline void markRemoved() {
            removed = true;
        };
};

class GUIManager {

    public:
        GUIManager();
        virtual ~GUIManager();
    private:
        FontInfo font;
        VAO quadVAO;
        std::vector<GUI> guis;
        uint8_t selected = null;
        float ratio = 2;
        Shader backgroundShader, textShader, dialShader;
        mat4 orthoMat;

    public:
        void init();
        void selectGUI( uint8_t guiid );
        void deselectGUI();
        uint8_t create();
        void remove( uint8_t &guiid );
        void draw();
        inline void setRatio( float ratio ) {
            this->ratio = ratio;
            glm_ortho( 0, ratio, 0, 1, 0, 1, orthoMat );
        };
        uint8_t cursorSelect( float x, float y );
        void setColor(const vec3 &primary, const vec3 &secondary);

        // Exposed GUI functions
        uint8_t addText( const vec4 &bounds, const char *text );
        uint8_t addInputText( const vec4 &bounds, const char *text, uint8_t charlimit );
        uint8_t addButton( const vec4 &bounds, const char *label );
        uint8_t addToggle( const vec4 &bounds, const char *label );
        uint8_t addSlide( const vec4 &bounds, uint32_t multiplier);
        uint8_t addKeyCapture( const vec4 &bounds );

        float getFloatValue(uint8_t guiid, uint8_t element);
        uint32_t getIntValue(uint8_t guiid, uint8_t element);
        void setText(uint8_t guiid, uint8_t element, string &text);

        void charInsert( char c );
        void charDelete();
        void keyPress(int32_t key, uint8_t modifiers);
        void inputArrowShift(int32_t amount);
        void inputArrow(int32_t position);

};

#endif // GUI_H

