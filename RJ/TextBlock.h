#pragma once

#ifndef __TextBlockH__
#define __TextBlockH__

#include "CompilerSettings.h"
#include "Utility.h"
#include "iUIComponent.h"
class TextManager;
struct TextManager::SentenceType;
using namespace std;

#define DEBUG_LOGINSTANCECREATION


// Class has no special alignment requirements
class TextBlock : public iUIComponent
{
public:
	TextBlock(void);
	~TextBlock(void);

	// Initialises the text block object
	Result Initialise(string code, TextManager *tm, TextManager::SentenceType *sentence, int maxlength);

	// Methods to set the text block contents
	void SetText(string text);
	void SetText(string text, float size);
	void SetText(const char *text);
	void SetText(const char *text, float size);

	// Method to peform a full update of the text block
	void UpdateTextBlock(const char *text, int positionX, int positionY, bool render, XMFLOAT4 textcolour, float size);

	// Methods to change the render state of the textblock
	void SetRenderActive(bool render);
	void ParentRenderStateChanged(bool parentstate);
	bool GetRenderActive(void) { return m_render; }
	bool IsActuallyVisible(void) { return (m_render && m_parentrender); }

	// Set the position of this text block
	void SetPosition(const INTVECTOR2 & pos);

	// Sets the colour of the text in this block
	void SetColour(const XMFLOAT4 & colour);

	// Methods to set/retrieve text block properties
	CMPINLINE string	GetCode(void) { return m_code; }
	CMPINLINE void		SetCode(string code) { m_code = code; }

	CMPINLINE float		GetSize(void) { return m_size; }
	const std::string &	GetText(void);
	INTVECTOR2			GetPosition(void);
	XMFLOAT4			GetTextColour(void);
	float				GetTextWidth(void);
	float				GetTextHeight(void);
	
	// Shutdown method to deallocate all resources assigned to this text block
	void				Shutdown(void);

private:
	string										m_code;

	TextManager *								m_textmanager;
	TextManager::SentenceType *					m_sentence;
	
	char *										m_textbuffer;
	std::string									m_string_textbuffer;	// Updated when a std::string 'text' value is requested
	int											m_maxlength;
	float										m_size;

	bool										m_render;
	bool										m_parentrender;


public:
	// Debug variables to log instance creation
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		static long inst_con;
		static long inst_des;
	#endif

};




#endif