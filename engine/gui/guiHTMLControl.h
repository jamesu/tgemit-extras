#pragma once


#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _TEXTURE_MANAGER_H_
#include "dgl/gTexManager.h"
#endif
#ifndef _GFONT_H_
#include "dgl/gFont.h"
#endif

#include <string>

class GuiToHTMLContainer;

namespace litehtml
{
class context;
class document;
typedef std::shared_ptr<document>   DocPtr;
}

class GuiHTMLControl : public GuiControl
{
private:
   typedef GuiControl Parent;

public:
   DECLARE_CONOBJECT(GuiHTMLControl);
   GuiHTMLControl();
   ~GuiHTMLControl();
   
   static void initPersistFields();

   //Parental methods
   bool onAdd();
   bool onWake();
   void onSleep();
   void inspectPostApply();
   
   void setContent(const char* value);

   void onRender(Point2I offset, const RectI &updateRect);
   
   litehtml::context* mDocContext;
   container_test* mDocContainer;
   litehtml::DocPtr mDoc;
   
   std::string mContent;
};
