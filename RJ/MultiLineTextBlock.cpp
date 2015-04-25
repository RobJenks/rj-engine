#include "ErrorCodes.h"
#include "FastMath.h"
#include "Utility.h"
#include "GameDataExtern.h"
#include "UserInterface.h"
#include "Render2DGroup.h"
#include "TextBlock.h"

#include "MultiLineTextBlock.h"

// Initialise constant values
const char *MultiLineTextBlock::BACKDROP_COMPONENT = "\\UI\\Common\\Content\\mltb_back.dds";

// Default constructor
MultiLineTextBlock::MultiLineTextBlock(void)
{
	// Set default values
	m_code = "";
	m_render = false;
	m_lines = NULL;
	m_mode = MultiLineTextBlock::OperationMode::IndividualLines;
	m_text = NullString;

	m_location = NULL_INTVECTOR2;
	m_zvalue = 0.0f;
	m_size = NULL_INTVECTOR2;
	m_linecount = 0;
	m_maxlinelength = 0;
	m_font = 0;
	m_fontsize = 0.0f;
	m_colour = NULL_VECTOR4;
	m_back = NULL;
}

// Initialise the component and set up all resources
Result MultiLineTextBlock::Initialise(Render2DGroup *parent, std::string code, MultiLineTextBlock::OperationMode mode, INTVECTOR2 location, float z, INTVECTOR2 size,
										int linecount, int maxlinelength, int font, float fontsize, D3DXVECTOR4 col, bool render)
{
	// Parameter check
	if (parent == NULL) return ErrorCodes::CannotInitialiseMLTBlockWithoutParent;
	if (code == NullString) return ErrorCodes::CannotInitialiseMLTBlockWithInvalidParameters;
	if (linecount < 1) linecount = 1;
	if (maxlinelength < 1) maxlinelength = 256;
	if (font < 0) font = 0;
	if (fontsize < 0.0f) fontsize = 1.0f;

	// Store key parameters
	m_code = code;
	m_parent = parent;
	m_mode = mode;
	m_location = location;
	m_zvalue = z;
	m_size = size;							// Either dimension can be 0, and will then be auto-calculated
	m_linecount = linecount;
	m_maxlinelength = maxlinelength;
	m_font = font;
	m_fontsize = fontsize;
	m_colour = col;
	m_render = render;

	// Allocate space for each line of the text block
	if (m_lines) { free(m_lines); m_lines = NULL; }
	m_lines = (TextBlock**)malloc(sizeof(TextBlock*) * m_linecount);
	if (!m_lines) return ErrorCodes::CouldNotAllocateMemoryForMLTBlock;

	// We now want to create a new text block for each of these lines
	TextBlock *tb; 
	INTVECTOR2 pos = INTVECTOR2(m_location.x + MultiLineTextBlock::TEXT_MARGIN, m_location.y + MultiLineTextBlock::TEXT_MARGIN);
	for (int i = 0; i < linecount; ++i)
	{
		// Attempt to create the line
		tb = D::UI->CreateTextBlock(concat(m_code)(".line")(i).str().c_str(), "", m_maxlinelength, m_font, pos, m_fontsize, m_colour, m_render);
		if (!tb) return ErrorCodes::CouldNotCreateLineWithinMLTBlock;

		// Store the line and add it to our parent render group 
		m_lines[i] = tb;
		m_parent->Components.TextBlocks.AddItem(tb->GetCode(), tb);
		
		// Slight hack; for one line, fill the line to max length with a medium-width character, test the size 
		// to determine control width (if required) , then revert back to null string
		if (i == 0 && m_size.x == 0)
		{
			std::string s = std::string(m_maxlinelength, 'o');
			tb->SetText(s);
			m_size.x = ((int)ceilf(tb->GetTextWidth()) + (2 * MultiLineTextBlock::TEXT_MARGIN));
			tb->SetText(NullString);
		}

		// Move to the next line
		pos.y += ((int)ceilf(tb->GetTextHeight()) + MultiLineTextBlock::LINE_SPACING);
	}

	// The final pos.y value minus its starting location (and plus lower margin) will be the height value for this control (if not already provided)
	if (m_size.y == 0)
		m_size.y = (pos.y - m_location.y) + MultiLineTextBlock::TEXT_MARGIN;

	// Set the component code, which will also update all sub-components
	SetCode(m_code);

	// Set the component render state, which will also update all sub-components
	SetRenderActive(m_render);

	// Create a backdrop component for the control
	std::string back_filename = concat(D::DATA)(MultiLineTextBlock::BACKDROP_COMPONENT).str();
	m_back = D::UI->NewComponent(concat(m_code)(".back").str(), back_filename.c_str(), m_location.x, m_location.y, m_zvalue, m_size.x, m_size.y);
	if (m_back)
	{
		// Set the render state based on this control state
		m_back->SetRenderActive(m_render);

		// Add this component to the render group
		m_parent->Components.Image2D.AddItem(m_back->GetCode(), m_back);
		m_parent->RegisterRenderableComponent(m_back);
	}

	// Return success
	return ErrorCodes::NoError;
}


// Set the text for this component.  Only valid where (mode == WordWrap)
void MultiLineTextBlock::SetText(const std::string & text)
{
	// Make sure we are in the correct mode for this function
	if (m_mode != MultiLineTextBlock::OperationMode::WordWrap) return;

	int nextchar = 0, wrap, count;
	int length = text.size();

	// Store the text locally
	m_text = text;

	// Populate each line with text until we run out of characters (or capacity)
	for (int i = 0; i < m_linecount; ++i)
	{
		if (!m_lines[i]) continue;

		// See if we have any more text to add to this line
		count = min((length - nextchar), m_maxlinelength);
		if (count > 0)
		{
			// Get the ideal word wrap point, then work backwards until we find a separator
			for (wrap = nextchar + count - 1; wrap > nextchar; --wrap)
				if (((text[wrap] >= 'A' && text[wrap] <= 'Z') || (text[wrap] >= 'a' && text[wrap] <= 'z')) == false) break;

			// Only use the wrap value if applicable; if there is no wrap point within this range then make no change and output as-is
			if (wrap > nextchar) count = (wrap - nextchar + 1);

			// Set the text for this line
			m_lines[i]->SetText(text.substr(nextchar, count));
		}
		else
		{
			if (m_lines[i]->GetText() != NullString) m_lines[i]->SetText("");
		}

		// Increment the next character to be processed by the number of characters we just processed
		nextchar += count;
	}
}

// Set the text for a particular line of this component.  Only valid where (mode == IndividualLines)
void MultiLineTextBlock::SetText(int line_number, const std::string & text)
{
	// Make sure we are in the correct mode for this function
	if (m_mode != MultiLineTextBlock::OperationMode::IndividualLines) return;

	// Make sure the line index is valid
	if (line_number < 0 || line_number >= m_linecount || m_lines[line_number] == NULL) return;

	// We will only transfer characters up to the maximum line limit
	if ((int)text.size() > m_maxlinelength)
	{
		m_lines[line_number]->SetText(text.substr(0, m_maxlinelength));
	}
	else
	{
		m_lines[line_number]->SetText(text);
	}
}


// Returns the text in this component
const std::string & MultiLineTextBlock::GetText(void)
{
	// If we are in individual-line mode then we first want to concatenate all lines and populate the local text property
	if (m_mode == MultiLineTextBlock::OperationMode::IndividualLines)
	{
		// Work from the bottom-up, so we can ignore any blank lines at the end and avoid multiple "\n"
		m_text = NullString;
		for (int i = m_linecount-1; i >= 0; --i)
		{
			// We want to add this text if (a) it is not "", or (b) we already have some text (and so are happy to add the mid-text "\n")
			if (m_lines[i] && (m_lines[i]->GetText() != NullString || m_text != NullString))
			{
				// Concatenate to the start of the overall string
				m_text = concat(m_lines[i]->GetText())("\n")(m_text).str();
			}
		}
	}

	// Return the local text property
	return m_text;
		
}

// Returns the text on one line of this component
const std::string & MultiLineTextBlock::GetText(int line)
{
	// Make sure the line index is valid
	if (line < 0 || line >= m_linecount || !m_lines[line])
		return NullString;
	else
		return m_lines[line]->GetText();
}

// Set the text colour for a particular line of this component
void MultiLineTextBlock::SetColour(int line_number, const D3DXVECTOR4 & colour)
{
	// Make sure the line index is valid
	if (line_number < 0 || line_number >= m_linecount || m_lines[line_number] == NULL) return;

	// Set the colour of this line 
	m_lines[line_number]->SetColour(colour);
}

// Set the text colour for all lines of this component
void MultiLineTextBlock::SetColour(const D3DXVECTOR4 & colour)
{
	// Iterate over all lines in the component
	for (int i = 0; i < m_linecount; ++i)
	{
		// Make sure the line component is still valid
		if (!m_lines[i]) continue;

		// Set the colour of this line
		m_lines[i]->SetColour(colour);
	}
}

// Set the position of the multi-line text block, updating all child components as necessary
void MultiLineTextBlock::SetPosition(INTVECTOR2 pos)
{
	TextBlock *tb;

	// Store the new component position
	m_location = pos;

	// Set the position of each line in turn
	for (int i = 0; i < m_linecount; ++i)
	{
		// Get a reference to this line and make sure it is still a valid component
		tb = m_lines[i]; if (!tb) continue;

		// Set the position of this line
		tb->SetPosition(pos);

		// Move to the next line
		pos.y += ((int)ceilf(tb->GetTextHeight()) + MultiLineTextBlock::LINE_SPACING);
	}

	// Update the backdrop component
	m_back->SetPosition(m_location.x, m_location.y);
}

// Sets the unique string code for this component
void MultiLineTextBlock::SetCode(std::string code)
{
	// Make sure the code is valid
	if (code == NullString) return;

	// Set the code for this component
	m_code = code;

	// Also set the code of each sub-component
	for (int i = 0; i < m_linecount; ++i)
	{
		if (m_lines[i]) m_lines[i]->SetCode(concat(m_code)(".line")(i).str().c_str());
	}
}

// Enables or disables rendering of this multi-line text block
void MultiLineTextBlock::SetRenderActive(bool render)
{
	// Store the new render value
	m_render = render;

	// Also set the render state of each sub-component
	if (m_back) m_back->SetRenderActive(render);
	for (int i = 0; i < m_linecount; ++i)
	{
		if (m_lines[i]) m_lines[i]->SetRenderActive(render);
	}

}

// Clears all text in the control
void MultiLineTextBlock::Clear(void)
{
	// Clear the local text property
	m_text = "";

	// Clear the contents of each line in the text block
	for (int i = 0; i < m_linecount; ++i)
	{
		if (m_lines[i]) m_lines[i]->SetText(NullString);
	}
}

// Returns the line at the specified ([0,0]-based) location within the control.  Returns -1 if no line was clicked.
int MultiLineTextBlock::GetLineAtLocation(const INTVECTOR2 & location) const
{
	// Make sure the point is valid in the x dimension; we can only be between 0 and size.x in local coordinates
	if (location.x < 0 || location.x > m_size.x) return -1;
	
	// Parameter check before retrieving info from lines
	if (m_linecount < 1 || m_lines == NULL || m_lines[0] == NULL) return -1;

	// We can now determine the line number using basic division.  Remove the margin first
	float y = (float)(location.y - MultiLineTextBlock::TEXT_MARGIN);
	float line_height = m_lines[0]->GetTextHeight() + (float)MultiLineTextBlock::LINE_SPACING;

	// Determine which line this should be by dividing through by the individual text block height
	int line = (int)floorf(y / line_height);

	// Assuming this line is valid (i.e. not out of bounds) then return it now
	if (line >= 0 && line < m_linecount) return line; else return -1;
}

// Shutdown method to deallocate all resources used by the component
void MultiLineTextBlock::Shutdown(void)
{
	// We want to remove all individual text blocks from the render group, where possible
	for (int i = 0; i < m_linecount; ++i)
	{
		// Make sure this is a valid line
		if (!m_lines[i]) continue;

		// Attempt to remove from the parent render group
		if (m_parent) m_parent->Components.TextBlocks.RemoveItem(m_lines[i]->GetCode());

		// Shut down the component and release all its resource.  NOTE: this may not deallocate the underlying sentence
		// object.  These are only deallocated by the text manager in one batch on shutdown.  Probably need to correct
		// this, unless the volume will be low enough that we simply tolerate the accumulation of memory
		m_lines[i]->Shutdown();
		SafeDelete(m_lines[i]);
	}

	// Also deallocate the control backdrop
	if (m_back)
	{
		if (m_parent)
		{
			m_parent->UnregisterRenderableComponent(m_back);
			m_parent->Components.Image2D.RemoveItem(m_back->GetCode());
		}
		m_back->Shutdown();
		SafeDelete(m_back);
	}

	// Deallocate any memory that was allocated specifically for this component
	SafeFree(m_lines);
}


// Default destructor
MultiLineTextBlock::~MultiLineTextBlock(void)
{

}

// Static methods to translate MLTB operation mode to/from text
MultiLineTextBlock::OperationMode MultiLineTextBlock::TranslateOperationModeFromString(const std::string & mode)
{
	// Comparisons are case-insensitive
	string s = mode; StrLowerC(s);

	if (s == "wordwrap")				return MultiLineTextBlock::OperationMode::WordWrap;

	// Default operation mode
	else								return MultiLineTextBlock::OperationMode::IndividualLines;	
}

// Static methods to translate MLTB operation mode to/from text
std::string	MultiLineTextBlock::TranslateOperationModeToString(MultiLineTextBlock::OperationMode mode)
{
	switch (mode)
	{
		case MultiLineTextBlock::OperationMode::WordWrap:			return "wordwrap";

		// Default operation mode
		default:													return "individuallines";
	}
}


