#pragma once

#ifndef __MultiLineTextBlockH__
#define __MultiLineTextBlockH__

#include <string>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"
#include "iUIComponent.h"
class Render2DGroup;
class TextBlock;


// This class has no special alignment requirements
class MultiLineTextBlock : public iUIComponent
{
public:

	// Possible modes of operation for the component
	enum OperationMode							{ IndividualLines = 0, WordWrap };

	// Static constants
	static const int							LINE_SPACING = 2;

	// Default constructor
	MultiLineTextBlock(void);

	// Initialise the component and set up all resources
	Result										Initialise(	Render2DGroup *parent, std::string code, MultiLineTextBlock::OperationMode mode, INTVECTOR2 location, float z,
															INTVECTOR2 size, int linecount, int maxlinelength, int font, float fontsize, XMFLOAT4 col, bool render);

	// Methods to retrieve the operation mode of this component (read-only)
	CMPINLINE OperationMode						GetOperationMode(void) const { return m_mode; }

	// Methods to retrieve and set the render state of this component
	CMPINLINE bool								GetRenderActive(void) const { return m_render; }
	void										SetRenderActive(bool render);

	// Set the text for this component.  Only valid where (mode == WordWrap)
	void										SetText(const std::string & text);

	// This control uses the extended SetCode method
	void										SetCodeEx(const std::string & code);

	// Set the text for a particular line of this component.  Only valid where (mode == IndividualLines)
	void										SetText(int line_number, const std::string & text);

	// Returns the text in this component
	const std::string &							GetText(void);

	// Returns the text on one line of this component
	const std::string &							GetText(int line);

	// Set the text colour for a particular line of this component
	void										SetColour(int line_number, const XMFLOAT4 & colour);

	// Set the text colour for all lines of this component
	void										SetColour(const XMFLOAT4 & colour);

	// Get or set the position of the multi-line text block, updating all child components as necessary
	CMPINLINE INTVECTOR2						GetPosition(void) const { return m_location; }
	void										SetPosition(INTVECTOR2 pos);

	// Gets the size of this text block; calculated based on the line count & max line length, so read-only
	CMPINLINE INTVECTOR2						GetSize(void) const { return m_size; }

	// Clears all text in the control
	void										Clear(void);

	// Returns the number of lines displayed in this multi-line text block
	CMPINLINE int								GetLineCount(void) const { return m_linecount; }

	// Returns the line at the specified ([0,0]-based) location within the control
	int											GetLineAtLocation(const INTVECTOR2 & location) const;

	// Shutdown method to deallocate all resources used by the component
	void										Shutdown(void);

	// Default destructor
	~MultiLineTextBlock(void);

	// Static methods to translate MLTB operation mode to/from text
	static OperationMode						TranslateOperationModeFromString(const std::string & mode);
	static std::string							TranslateOperationModeToString(OperationMode mode);

	// Static constants
	static const int							TEXT_MARGIN = 6;
	static const std::string					BACKDROP_COMPONENT;

protected:

	Render2DGroup *								m_parent;
	OperationMode								m_mode;

	std::string *								m_linecodes;
	TextBlock **								m_lines;
	std::string									m_text;

	INTVECTOR2									m_location;
	float										m_zvalue;
	INTVECTOR2									m_size;
	int											m_linecount;
	std::string::size_type						m_maxlinelength;
	int											m_font;
	float										m_fontsize;
	XMFLOAT4									m_colour;
	Image2D *									m_back;
	std::string									m_backcode;

};




#endif



