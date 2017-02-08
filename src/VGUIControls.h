#pragma once
#include "Utils.h"
#include "Config.h"
#include "Array.h"
#include "Utils.h"

struct SProperty;

namespace GUI
{
	class Panel;

	typedef void (*HoverCallbackFn)(Panel*, int, int);
	typedef void (*ClickCallbackFn)(Panel*, int, int);
	typedef void (*FocusCallbackFn)(Panel*);
	typedef void (*EnteredCallbackFn)(Panel*);

	enum 
	{
		SIZE_PIXEL,
		SIZE_PERCENT
	};

	enum 
	{
		TOP,
		RIGHT,
		BOTTOM,
		LEFT
	};

	enum
	{
		CURSOR_NONE		= 1,
		CURSOR_NORMAL	= 2,
		CURSOR_HAND		= 3,
		CURSOR_CROSS	= 4,
		CURSOR_BEAM		= 5,
	};

	enum
	{
		ALIGN_TOP		= 1,
		ALIGN_BOTTOM	= 2,
		ALIGN_LEFT		= 4,
		ALIGN_RIGHT		= 8,
	};

	//////////////////////////////////////////////////////////////////////////
	struct Style
	{
		int			x, y;						// desired position
		int			width, height;			
		int			widthType, heightType;
		SRGBA		background;
		SRGBA		color;						// font color
		SRGBA		borders[4];					// border colors (if any)
		int			padding[4];
		int			margin[4];
		int			align;
		int			zIndex;
		bool		hidden;
		int			cursor;		
		int			lineHeight;
		string		image;
		int			imageX;
		int			imageY;
		int			imageWidth;
		int			imageHeight;
		bool		shadow;

		Style()
		{
			x = 0;
			y = 0;
			width = 100;
			widthType = SIZE_PERCENT;
			height = 0;
			heightType = SIZE_PIXEL;
			background = TRANSPARENT;
			color = WHITE;
			align = ALIGN_TOP | ALIGN_LEFT;
			zIndex = 0;
			hidden = false;
			cursor = CURSOR_NONE;
			lineHeight = 12;
			imageX = imageY = 0;
			imageWidth = 0;
			imageHeight = 0;
			shadow = false;

			for (int i=0; i<4; i++)
			{
				borders[i] = TRANSPARENT;
				padding[i] = 0;
				margin[i] = 0;
			}
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class Panel
	{
		TYPE("Panel");
	public:
		Panel();
		virtual ~Panel();

		void				Draw();
		virtual void		DrawElement();
		void				Remove();
		void				Empty();
		void				Add(Panel *child);
		void				SetClass(string c);
		void				ApplyClassStyles();

		static Style		ReadStyle(ConfigFile &config, string group);
		
		void				ApplyStyle( ConfigFile &config, string group );
		void				SetFocus(bool value, bool recursive = false);

	public:
		int					x, y, width, height;		// actual position and coordinates
		Style				style;
		bool				visible;
		string				text;
		
		static Style		defaultStyle;
		CArray<string>		classes;
		SProperty			*pBind;

		bool				isHover;
		bool				isFocus;

		bool				focusable;

		HoverCallbackFn		onHover;
		ClickCallbackFn		onClick;
		FocusCallbackFn		onFocus;
		FocusCallbackFn		onBlur;
		EnteredCallbackFn	onEnter;

		Panel				*pParent;
		CArray<Panel*>		children;
	};

	//////////////////////////////////////////////////////////////////////////
	class Input : public Panel
	{
		TYPE("Input");
	public:
		Input();
		virtual ~Input();

		virtual void		DrawElement();

		static Style		defaultStyle;
	};

	//////////////////////////////////////////////////////////////////////////
	class CheckBox : public Panel 
	{
		TYPE("CheckBox");
	public:
		CheckBox();
		
		virtual void		DrawElement();

		static Style		defaultStyle;
	};

	//////////////////////////////////////////////////////////////////////////
	class FileInput : public Panel
	{
		TYPE("FileInput");
	public:
		FileInput();
	};
}
