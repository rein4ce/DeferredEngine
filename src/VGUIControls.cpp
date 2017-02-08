#include "platform.h"
#include "VGUIControls.h"
#include "Engine.h"
#include "Renderer.h"
#include "GeometryRenderer.h"
#include "VGUI.h"
#include "Entity.h"
#include "Properties.h"
#include <boost/regex.hpp>
#include <boost\algorithm\string\case_conv.hpp>
#include <boost\algorithm\string\split.hpp>
#include <boost\algorithm\string\classification.hpp>
#include "keydefs.h"
#include "Input.h"

extern VoidFont g_DefaultFont;

GUI::Style GUI::Panel::defaultStyle;
GUI::Style GUI::Input::defaultStyle;
GUI::Style GUI::CheckBox::defaultStyle;


namespace GUI
{
	GUI::Input *gFocusedInput = NULL;


	void InputOnEnter()
	{
		string text = gInput.GetTypingText();
		Debug("Typed: %s", text.c_str());

		if (!gFocusedInput)	return;
		
		if (gFocusedInput->pBind)	
		{
			gFocusedInput->pBind->SetValue(text);	
			gFocusedInput->text = "";
		}
		else
			gFocusedInput->text = text;		

		if (gFocusedInput->onEnter) gFocusedInput->onEnter(gFocusedInput);
	}

	void InputOnFocus(GUI::Panel *panel)
	{
		GUI::Input *input = (GUI::Input*)panel;
		gFocusedInput = input;

		Debug("Focused on input");
		string placeholder = input->pBind ? input->pBind->GetValue() : input->text;
		gInput.BeginTyping(placeholder.c_str(), InputOnEnter);
	}

	void InputOnBlur(GUI::Panel *panel)
	{
		GUI::Input *input = (GUI::Input*)panel;
		input->text = "";
		gFocusedInput = NULL;
	}

	void CheckBoxClick(Panel *panel, int x, int y)
	{
		if (!panel->pBind) return;
		if (panel->pBind->GetValue() == "true")
			panel->pBind->SetValue("false");
		else
			panel->pBind->SetValue("true");
	}

	void FileInputClick(Panel *panel, int x, int y)
	{		
		Debug("File input clicked");

		OPENFILENAME desc;
		char file[256];
		ZeroMemory(&desc, sizeof(desc));
		desc.hwndOwner = gEngine.GetHWND();
		desc.lStructSize = sizeof(desc);
		desc.lpstrFile = file;
		desc.lpstrFile[0] = '\0';
		desc.nMaxFile = sizeof(file);
		desc.lpstrInitialDir = NULL;
		desc.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&desc))
		{
			if (panel->pBind)
			{
				panel->pBind->SetValue(file);
				panel->text = GetPathFileName(file);
			}
			else
			{
				panel->text = file;
			}		

			if (panel->onEnter) panel->onEnter(panel);
		}
	}


	GUI::Panel::Panel()
	{
		pParent = NULL;
		visible = true;
		isHover = false;
		isFocus = false;
		focusable = false;
		onClick = NULL;
		onHover = NULL;
		onFocus = NULL;
		onBlur = NULL;
		onEnter = NULL;
		pBind = NULL;

		style = GUI::Panel::defaultStyle;		
	}

	void Panel::ApplyClassStyles()
	{		
		for (int j=0; j<classes.Size(); j++)
			ApplyStyle(gVGUI.styles, classes[j]);

		for (int i=0; i<children.Size(); i++)
			children[i]->ApplyClassStyles();
	}

	GUI::Panel::~Panel()
	{
		trace("Panel destroyed");
	}

	//////////////////////////////////////////////////////////////////////////
	void GUI::Panel::Draw()
	{
		x = style.x;
		y = style.y; 

		int parentWidth = pParent ? pParent->width : gEngine.width;
		int parentHeight = pParent ? pParent->height : gEngine.height;

		if (style.widthType == SIZE_PIXEL)	width = style.width;
		if (style.heightType == SIZE_PIXEL)	height = style.height;

		if (style.widthType == SIZE_PERCENT)	
			width = ((float)style.width / 100) * parentWidth 
				- style.margin[LEFT] - style.margin[RIGHT]
				- (	pParent ? pParent->style.padding[LEFT] + pParent->style.padding[RIGHT] : 0);
		if (style.heightType == SIZE_PERCENT)	
			height = ((float)style.height / 100) * parentHeight
			- style.margin[TOP] - style.margin[BOTTOM]
			- (pParent ? pParent->style.padding[TOP] + pParent->style.padding[BOTTOM] : 0);

		// apply margins
		if (style.align & ALIGN_LEFT)	x += style.margin[LEFT]; else
		if (style.align & ALIGN_RIGHT)	x -= style.margin[RIGHT];
		if (style.align & ALIGN_TOP)	y += style.margin[TOP]; else
		if (style.align & ALIGN_BOTTOM)	y -= style.margin[BOTTOM];
		
		if (pParent)
		{
			// apply parent position and padding
			if (style.align & ALIGN_LEFT)	x += pParent->x + pParent->style.padding[LEFT]; else
			if (style.align & ALIGN_RIGHT)	x += pParent->x + pParent->width - pParent->style.padding[RIGHT] - width;
			if (style.align & ALIGN_TOP)	y += pParent->y + pParent->style.padding[TOP]; else
			if (style.align & ALIGN_BOTTOM)	y += pParent->y + pParent->height - pParent->style.padding[BOTTOM] - height;		
		}

		// check if hover, and if yes, trigger hover event
		bool oldHover = isHover;
		isHover = gInput.IsMouseInRect(x, y, width, height);

		if (isHover && oldHover != isHover && onHover)
			onHover(this, gInput.GetMouseX()-x, gInput.GetMouseY()-y);

		// check for click
		if (isHover && gInput.WasKeyPressed(K_MOUSE1))
		{
			if (focusable) 
			{
				isFocus = true;
				if (onFocus) onFocus(this);
			}
			if (onClick) onClick(this, gInput.GetMouseX()-x, gInput.GetMouseY()-y);
		}

		// if panel is not visible, don't continue
		if (!visible) return;

		if (!style.hidden)
		{
			DrawElement();
		}

		if (isHover && style.cursor != CURSOR_NONE)
		{
			gVGUI.cursor = style.cursor;	
		}

		// draw all children
		for (int i=0; i<children.Size(); i++)
			children[i]->Draw();
	}

	//////////////////////////////////////////////////////////////////////////
	void GUI::Panel::DrawElement()
	{
		// draw the background (only if visible)
		float z = style.zIndex ? style.zIndex : gVGUI.GetNextZindex();

		if (style.background.a > 0)
		{
			SRGBA bgcolor = style.background;
			if (style.image.size() > 0)	
			{
				gRenderer.pGeometryRenderer->SetTexture(CTexture::Get(style.image));
				if (isHover) bgcolor.a = 255;
			}

			if (onClick && isHover && bgcolor.a < 255) bgcolor.a += 20;

			gRenderer.pGeometryRenderer->DrawQuad(Vector3(x, y, z), Vector3(x+width, y+height, z), bgcolor);
			if (style.image.size() > 0) 
			{
				gRenderer.pGeometryRenderer->Render();
				gRenderer.pGeometryRenderer->SetNoTexture();
				gRenderer.pGeometryRenderer->ortho = true;
			}
		}

		// draw borders
		z = style.zIndex ? style.zIndex+0.0001f : gVGUI.GetNextZindex();

		// if has focus, draw white border
		if (isFocus)
		{
			gRenderer.pGeometryRenderer->DrawLine(Vector3(x-1, y, z), Vector3(x+width, y, z), WHITE);
			gRenderer.pGeometryRenderer->DrawLine(Vector3(x, y+height, z), Vector3(x+width, y+height, z), WHITE);
			gRenderer.pGeometryRenderer->DrawLine(Vector3(x, y, z), Vector3(x, y+height, z), WHITE);
			gRenderer.pGeometryRenderer->DrawLine(Vector3(x+width, y, z), Vector3(x+width, y+height, z), WHITE);
		}
		else
		{
			if (style.borders[TOP].a > 0)		gRenderer.pGeometryRenderer->DrawLine(Vector3(x-1, y, z), Vector3(x+width, y, z), style.borders[TOP]);
			if (style.borders[BOTTOM].a > 0)	gRenderer.pGeometryRenderer->DrawLine(Vector3(x, y+height, z), Vector3(x+width, y+height, z), style.borders[BOTTOM]);
			if (style.borders[LEFT].a > 0)		gRenderer.pGeometryRenderer->DrawLine(Vector3(x, y, z), Vector3(x, y+height, z), style.borders[LEFT]);
			if (style.borders[RIGHT].a > 0)		gRenderer.pGeometryRenderer->DrawLine(Vector3(x+width, y, z), Vector3(x+width, y+height, z), style.borders[RIGHT]);
		}

		// draw text
		if (pBind || text.size())
		{
			string value = text.size() > 0 ? text : (pBind ? pBind->GetValue() : "");
			int fontSize = 12;
			gVGUI.AddTextMessageEx(x + style.padding[LEFT], y + style.padding[TOP] + (style.lineHeight-fontSize)/2, style.color, 0, style.shadow, style.color.a, value.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void GUI::Panel::Remove()
	{
		if (pParent) pParent->children.Remove(this);
		for (int i=0; i<children.Size(); i++)	
			children[i]->Remove();
		children.PurgeAndDeleteElements();
	}

	//////////////////////////////////////////////////////////////////////////
	void GUI::Panel::Add(Panel *child)
	{
		children.AddToTail(child);
		child->pParent = this;
	}

	//////////////////////////////////////////////////////////////////////////
	void GUI::Panel::ApplyStyle( ConfigFile &config, string group )
	{
		if (config[group]["x"].set)					style.x = config[group]["x"].GetInteger();
		if (config[group]["y"].set)					style.y = config[group]["y"].GetInteger();
		if (config[group]["text"].set)				text = config[group]["text"].GetString();
		if (config[group]["background"].set)		style.background = config[group]["background"].GetColor();
		if (config[group]["color"].set)				style.color = config[group]["color"].GetColor();		
		if (config[group]["cursor"].set)			style.cursor = config[group]["cursor"].GetInteger();
		if (config[group]["lineHeight"].set)		style.lineHeight = config[group]["lineHeight"].GetInteger();
		if (config[group]["shadow"].set)			style.shadow = config[group]["shadow"].GetBool();

		if (config[group]["image"].set)				style.image = config[group]["image"].GetString();
		if (config[group]["imageX"].set)			style.imageX = config[group]["imageX"].GetInteger();
		if (config[group]["imageY"].set)			style.imageY = config[group]["imageY"].GetInteger();
		if (config[group]["imageWidth"].set)		style.imageWidth = config[group]["imageWidth"].GetInteger();
		if (config[group]["imageHeight"].set)		style.imageHeight = config[group]["imageHeight"].GetInteger();

		// align
		if (config[group]["align"].set)
		{
			string align =config[group]["align"].GetString();
			style.align = 0;
			if (align.find("top") != string::npos) style.align |= ALIGN_TOP;
			if (align.find("bottom") != string::npos) style.align |= ALIGN_BOTTOM;
			if (align.find("left") != string::npos) style.align |= ALIGN_LEFT;
			if (align.find("right") != string::npos) style.align |= ALIGN_RIGHT;
		}

		// size
		if (config[group]["width"].set)
		{
			string value = config[group]["width"].GetString();
			boost::regex reNum("^(\\d+)(px|%)");
			boost::match_results<string::const_iterator> what;
			if (boost::regex_match(value, what, reNum))
			{
				style.width = atoi(string(what[1].first, what[1].second).c_str());
				if (value.find("px") != string::npos) style.widthType = SIZE_PIXEL;
				if (value.find("%") != string::npos) style.widthType = SIZE_PERCENT;
			}
		}

		if (config[group]["height"].set)
		{
			string value = config[group]["height"].GetString();
			boost::regex reNum("^(\\d+)(px|%)");
			boost::match_results<string::const_iterator> what;
			if (boost::regex_match(value, what, reNum))
			{
				style.height = atoi(string(what[1].first, what[1].second).c_str());
				if (value.find("px") != string::npos) style.heightType = SIZE_PIXEL;
				if (value.find("%") != string::npos) style.heightType = SIZE_PERCENT;
			}
		}

		// border
		if (config[group]["border"].set)			
		{
			style.borders[0] =
				style.borders[1] = 
				style.borders[2] =
				style.borders[3] = config[group]["border"].GetColor();
		}
		if (config[group]["borderTop"].set)			style.borders[TOP] = config[group]["borderTop"].GetColor();
		if (config[group]["borderBottom"].set)		style.borders[BOTTOM] = config[group]["borderBottom"].GetColor();
		if (config[group]["borderRight"].set)		style.borders[RIGHT] = config[group]["borderRight"].GetColor();
		if (config[group]["borderLeft"].set)		style.borders[LEFT] = config[group]["borderLeft"].GetColor();

		// margin
		if (config[group]["margin"].set)			
		{
			style.margin[0] =
				style.margin[1] = 
				style.margin[2] =
				style.margin[3] = config[group]["margin"].GetInteger();
		}
		if (config[group]["marginTop"].set)			style.margin[TOP] = config[group]["marginTop"].GetInteger();
		if (config[group]["marginBottom"].set)		style.margin[BOTTOM] = config[group]["marginBottom"].GetInteger();
		if (config[group]["marginRight"].set)		style.margin[RIGHT] = config[group]["marginRight"].GetInteger();
		if (config[group]["marginLeft"].set)		style.margin[LEFT] = config[group]["marginLeft"].GetInteger();

		// padding
		if (config[group]["padding"].set)			
		{
			style.padding[0] =
				style.padding[1] = 
				style.padding[2] =
				style.padding[3] = config[group]["padding"].GetInteger();
		}
		if (config[group]["paddingTop"].set)		style.padding[TOP] = config[group]["paddingTop"].GetInteger();
		if (config[group]["paddingBottom"].set)		style.padding[BOTTOM] = config[group]["paddingBottom"].GetInteger();
		if (config[group]["paddingRight"].set)		style.padding[RIGHT] = config[group]["paddingRight"].GetInteger();
		if (config[group]["paddingLeft"].set)		style.padding[LEFT] = config[group]["paddingLeft"].GetInteger();	
	}

	//////////////////////////////////////////////////////////////////////////
	void GUI::Panel::Empty()
	{
		for (int i=0; i<children.Size(); i++)	
			children[i]->Remove();
	}

	//////////////////////////////////////////////////////////////////////////
	void GUI::Panel::SetFocus(bool value, bool recursive)
	{
		this->isFocus = value;
		if (value && onFocus) onFocus(this);
		if (!value && onBlur) onBlur(this);

		if (recursive)
		{
			for (int i=0; i<children.Size(); i++)	
				children[i]->SetFocus(value, recursive);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	GUI::Input::Input()
	{
		style = GUI::Input::defaultStyle;		
		focusable = true;

		onFocus = InputOnFocus;
		onBlur = InputOnBlur;
	}

	GUI::Input::~Input()
	{

	}

	void GUI::Input::DrawElement()
	{
		if (gFocusedInput == this)
		{
			text = gInput.GetTypingText();
			if (text.size() == 0) text = " ";
		}

		GUI::Panel::DrawElement();

		// draw caret
		if (gFocusedInput == this)
		{	
			float sx, sy = 12;
			float z = style.zIndex ? style.zIndex+0.001f : gVGUI.GetNextZindex();
			sx = strlen(gInput.GetTypingText())*6;
			gRenderer.pGeometryRenderer->DrawLine(Vector3(x+style.padding[LEFT]+sx+2, y+style.padding[TOP], z), Vector3(x+style.padding[LEFT]+sx+2, y+style.padding[TOP]+sy, z), WHITE);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	CheckBox::CheckBox()
	{
		style = CheckBox::defaultStyle;		

		onClick = CheckBoxClick;
	}

	void CheckBox::DrawElement()
	{
		text = " ";
		GUI::Panel::DrawElement();

		if (pBind)
		{
			if (pBind->GetValue() == "true")
			{
				int pad = 2;
				float z = style.zIndex ? style.zIndex+0.001f : gVGUI.GetNextZindex();
				gRenderer.pGeometryRenderer->DrawQuad(Vector3(x+pad, y+pad, z), Vector3(x+width-pad-1, y+height-pad-1, z), style.color);
			}
		}
	}

	FileInput::FileInput()
	{
		style = Input::defaultStyle;		

		focusable = false;
		style.color = SRGBA(255,255,255,100);
		style.width = 150;		
		style.padding[RIGHT] = 0;
		
		Panel *btn = new Panel();
		btn->style.align = ALIGN_TOP | ALIGN_RIGHT;
		btn->style.margin[TOP] = -style.padding[TOP]-1;
		btn->style.margin[LEFT] = -style.padding[LEFT]-1;
		btn->style.padding[LEFT] = 6;		
		btn->style.padding[TOP] = 3;		
		btn->style.width = 25;
		btn->style.widthType = SIZE_PIXEL;
		btn->style.height = style.height+1;
		btn->text = "...";
		btn->style.background = BLACK;
		btn->style.cursor = CURSOR_HAND;
		this->Add(btn);
	
		onClick = FileInputClick;
	}

	//////////////////////////////////////////////////////////////////////////
	Style Panel::ReadStyle( ConfigFile &config, string group )
	{
		Panel panel;

		panel.ApplyStyle(config, group);
		return panel.style;		
	}

	void Panel::SetClass( string c )
	{
		vector<string> strs;
		boost::split(strs, c, boost::is_any_of(" "));

		for (int i=0; i<strs.size(); i++)
			classes.Add(strs[i]);

		ApplyClassStyles();
	}
}