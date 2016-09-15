#include "ErrorCodes.h"
#include "TextManager.h"

#include "TextBlock.h"
#include "SentenceType.h"
using namespace std;

#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
	long TextBlock::inst_con = 0;
	long TextBlock::inst_des = 0;
#endif


Result TextBlock::Initialise(string code, TextManager *tm, SentenceType *sentence, int maxlength)
{
	// Parameter check
	if (code.empty() || !tm || !sentence) return ErrorCodes::CannotInitialiseTextBlockWithInvalidParameters;

	// Store the parameters that have been provided
	m_code = code;
	m_textmanager = tm;
	m_sentence = sentence;
	m_maxlength = (maxlength > 0 ? maxlength : 256);

	// Rendering state; considers both our and the parent group's render state
	m_render = m_parentrender = true;

	// Allocate space for the text buffer
	m_textbuffer = new (nothrow) char[m_maxlength+1];		// Always allow +1 char for the null-terminator
	if (!m_textbuffer) return ErrorCodes::CouldNotAllocateSpaceForTextBlockBuffer;
	memset(m_textbuffer, 0, sizeof(char) * (m_maxlength + 1));

	// Return success
	return ErrorCodes::NoError;
}

// Sets the text block based on a std::string input.  Calls one overloaded method (that also calls another) to complete default params
void TextBlock::SetText(string text)
{
	SetText(text, m_size);
}

// Sets the text block based on a std::string input.  Calls the overloaded method with a cstring
void TextBlock::SetText(string text, float size)
{
	// Convert to a c-string and call the overloaded method
	const char *c = text.c_str();
	SetText(c, size);
}

// Sets the text in this block, via the overloaded function incorporating text size
void TextBlock::SetText(const char *text)
{
	SetText(text, m_size);
}

// Sets the text in this block, updating buffers etc as required
void TextBlock::SetText(const char *text, float size)
{
	// Store this text in the local string buffer.  Zero out first in case we are setting a shorter replacement
	// string, and also to ensure that null terminators are correctly set at the end of the string
	memset(m_textbuffer, 0, m_maxlength+1);
	strncpy(m_textbuffer, text, m_maxlength);
	m_size = size;

	// Now call the text manager update method to refresh vertex buffers etc
	m_textmanager->SetSentenceText(m_sentence, m_textbuffer, m_size);
}

// Sets the colour of the text in this block
void TextBlock::SetColour(const XMFLOAT4 & colour)
{
	m_textmanager->SetSentenceColour(m_sentence, colour);
}

// Performs a full update of the text block, including characteristics such as position, size and colour
void TextBlock::UpdateTextBlock(const char *text, int positionX, int positionY, bool render, XMFLOAT4 textcolour, float size)
{
	// Store this text and text size in the local string buffer
	SetText(text, size);

	// Call the text manager update method to invoke these changes (ignoring the return value)
	m_textmanager->UpdateSentence(m_sentence, m_textbuffer, positionX, positionY, render, textcolour, m_size);
}

// Set the position of this text block
void TextBlock::SetPosition(const INTVECTOR2 & pos)
{
	// Call the main text manager update method, with only the position parameters updated.  We need to perform the full
	// update so that it updates the vertex buffers correctly
	if (m_sentence) m_textmanager->UpdateSentence(m_sentence, m_textbuffer, pos.x, pos.y, m_render, m_sentence->colour, m_size);
}

// Enables or disables rendering of this text block.  Avoids a full update via UpdateSentence(...)
void TextBlock::SetRenderActive(bool render)
{
	// Store the new render value
	m_render = render;

	// Set actual rendering to also consider the render state of the parent group
	m_textmanager->SetSentenceRendering(m_sentence, (m_render && m_parentrender) );
}

void TextBlock::ParentRenderStateChanged(bool parentstate)
{
	// Store the new parent render state
	m_parentrender = parentstate;

	// Re-evaluate the render state of this component based on the new parent state by calling the component render state method
	SetRenderActive(m_render);
}
	
const std::string & TextBlock::GetText(void)
{
	if (m_textbuffer) {
		m_string_textbuffer = m_textbuffer;
	} else {
		m_string_textbuffer = "";
	}

	return m_string_textbuffer;
}
	
INTVECTOR2 TextBlock::GetPosition(void)
{
	return (INTVECTOR2(m_sentence->x, m_sentence->y));
}

XMFLOAT4	TextBlock::GetTextColour(void)
{
	return m_sentence->colour;
}

float TextBlock::GetTextWidth(void)
{
	return (m_sentence->sentencewidth);
}

float TextBlock::GetTextHeight(void)
{
	return (m_sentence->sentenceheight);
}

// Deallocates all resources and disposes of the text block
void TextBlock::Shutdown(void)
{
	// Debug purposes: record the number of elements being destructed
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_des;
	#endif

	// Deallocate the text buffer
	if (m_textbuffer) { delete m_textbuffer; m_textbuffer = NULL; }

	// We do NOT need to deallocate the sentence object; shutdown method of the text manager 
	// will dispose of all sentence resources
}


TextBlock::TextBlock(void)
{
	// Debug purposes: record the number of elements being created
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_con;
	#endif

	m_code = "";
	m_textmanager = NULL;
	m_sentence = NULL;
	m_textbuffer = NULL;
	m_maxlength = 1;
	m_size = 1.0f;
}

TextBlock::~TextBlock(void)
{
}
