#include "guiHTMLControl.h"
#include "gui/core/guiCanvas.h"
#include "dgl/dgl.h"
#include "litehtml.h"
#include "core/stringBuffer.h"
#include "core/fileStream.h"
#include <cctype>

using namespace litehtml;

/*
 
 GuiHTMLControl TODO:
 
 - Need some way of drawing rounded rects in DGL for borders
 - Use stencil buffer for rounded border clipping
 - TGE font API needs to be able to generate italic/bold/strikethrough/underline styles
 - Need to load textures via TextureHandle
 - Need to complete GuiControl element wrapping code
 
 */

class TrackedHTMLElement;

class GuiToHTMLContainer :	public litehtml::document_container
{
public:
	container_test();
	virtual ~container_test();
   
   Vector<Resource<GFont>> mLoadedFonts;
   GuiHTMLControl* mControl;
   Point2I mOffset;
   Vector<RectI> mClipStack;
   RectI mLastClipRect;
   U32 mLastControlId;
   
   Vector<TrackedHTMLElement*> mTrackedElements;

	virtual litehtml::uint_ptr			create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm) override;
	virtual void						delete_font(litehtml::uint_ptr hFont) override;
	virtual int							text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) override;
	virtual void						draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
	virtual int							pt_to_px(int pt) override;
	virtual int							get_default_font_size() const override;
	virtual const litehtml::tchar_t*	get_default_font_name() const override;
	virtual void 						load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready) override;
	virtual void						get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz) override;
	virtual void						draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) override;
	virtual void						draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
	virtual void 						draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
	virtual std::shared_ptr<litehtml::element>	create_element(const litehtml::tchar_t *tag_name,
																 const litehtml::string_map &attributes,
																 const std::shared_ptr<litehtml::document> &doc) override;
	virtual void						get_media_features(litehtml::media_features& media) const override;
	virtual void						get_language(litehtml::tstring& language, litehtml::tstring & culture) const override;
	virtual void 						link(const std::shared_ptr<litehtml::document> &ptr, const litehtml::element::ptr& el) override;


	virtual	void						transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
	virtual void						set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
	virtual void						del_clip() override;

	virtual void						make_url( const litehtml::tchar_t* url, const litehtml::tchar_t* basepath, litehtml::tstring& out );


	virtual	void 						set_caption(const litehtml::tchar_t* caption) override;
	virtual	void 						set_base_url(const litehtml::tchar_t* base_url) override;
	virtual void						on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
	virtual	void						set_cursor(const litehtml::tchar_t* cursor) override;
	virtual void						import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
	virtual void						get_client_rect(litehtml::position& client) const override;
};

class TrackedHTMLElement : public litehtml::element
{
public:
   GuiControl* mControl;
   
   TrackedHTMLElement(const std::shared_ptr<litehtml::document>& doc) :
   element(doc)
   {
      mControl = NULL;
   }
   
   ~TrackedHTMLElement()
   {
      if (mControl)
         mControl->unregisterObject();
   }
};

// TOFIX: tidy up

container_test::container_test() {}
container_test::~container_test() {}
litehtml::uint_ptr container_test::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm) {
   
  Resource<GFont> theFont = GFont::create(faceName, size, Con::getVariable("$GUI::fontCacheDirectory"));
   
   if (theFont)
   {
      PlatformFont::CharInfo ch = theFont->getCharInfo('x');
      fm->ascent = theFont->getAscent();
      fm->descent = theFont->getDescent();
      fm->height = theFont->getHeight();
      fm->x_height = ch.height * (1.0f / theFont->getTextureScale());
      return (litehtml::uint_ptr)theFont.getPtr();
   }
   
  if (fm) {
    fm->ascent = 10;
    fm->descent = 5;
    fm->height = 10 + 5;
    fm->x_height = 3;
  }
  return (litehtml::uint_ptr)0;
}
void container_test::delete_font(litehtml::uint_ptr hFont) {
   auto el = std::find_if(mLoadedFonts.begin(), mLoadedFonts.end(), [hFont](Resource<GFont> &el){
      return ((litehtml::uint_ptr)el.getPtr()) == hFont;
   });
   
   if (el != mLoadedFonts.end())
   {
      mLoadedFonts.erase(el);
   }
}
int container_test::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) {
   GFont* theFont = (GFont*)hFont;
   return theFont ? (theFont->getStrWidthPrecise(text)) : 0;
}
void container_test::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) {
   dglSetBitmapModulation(ColorI(color.red, color.green, color.blue, color.alpha));
   dglDrawText((GFont*)hFont, mOffset + Point2I(pos.x, pos.y), text);
}
int container_test::pt_to_px(int pt) { return (int)((double)pt * 96 / 72.0); }
int container_test::get_default_font_size() const { return 16; }
const litehtml::tchar_t* container_test::get_default_font_name() const { return _t("Times New Roman"); }
void container_test::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) {}
void container_test::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready) {
   
}
void container_test::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz) {
   
}
void container_test::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) {
   RectI destRect(mOffset + Point2I(bg.clip_box.x, bg.clip_box.y), Point2I(bg.clip_box.width, bg.clip_box.height));
   if (bg.image.empty()) {
      dglDrawRectFill(destRect, ColorI(bg.color.red, bg.color.green, bg.color.blue, bg.color.alpha));
   } else {
      // TODO
   }
}
void container_test::make_url(const litehtml::tchar_t* url, const litehtml::tchar_t* basepath, litehtml::tstring& out) { out = url; }
void container_test::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) {
   
   int bdr_top      = 0;
   int bdr_bottom   = 0;
   int bdr_left   = 0;
   int bdr_right   = 0;
   
   //std::cout << "   uint ptr: " << +hdc << std::endl;
   //std::cout << "   this ptr: " << +this << std::endl;

   if(borders.top.width != 0 && borders.top.style > litehtml::border_style_hidden)
   {
      bdr_top = (int) borders.top.width;
   }
   if(borders.bottom.width != 0 && borders.bottom.style > litehtml::border_style_hidden)
   {
      bdr_bottom = (int) borders.bottom.width;
   }
   if(borders.left.width != 0 && borders.left.style > litehtml::border_style_hidden)
   {
      bdr_left = (int) borders.left.width;
   }
   if(borders.right.width != 0 && borders.right.style > litehtml::border_style_hidden)
   {
      bdr_right = (int) borders.right.width;
   }
   
   
   // TODO: styled borders in dgl
   
   if (bdr_bottom)
   {
      RectI destRect(mOffset + Point2I(draw_pos.left(), draw_pos.top()), Point2I(draw_pos.width, draw_pos.height));
      dglDrawRect(destRect, ColorI(borders.top.color.red, borders.top.color.green, borders.top.color.blue, borders.top.color.alpha));
   }
}

void container_test::set_caption(const litehtml::tchar_t* caption){};    //: set_caption
void container_test::set_base_url(const litehtml::tchar_t* base_url){};  //: set_base_url
void container_test::link(const std::shared_ptr<litehtml::document>& ptr, const litehtml::element::ptr& el) {}
void container_test::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) {}  //: on_anchor_click
void container_test::set_cursor(const litehtml::tchar_t* cursor) {}                                      //: set_cursor
void container_test::transform_text(litehtml::tstring& text, litehtml::text_transform tt) {
   if(text.empty()) return;
   
   StringBuffer theBuffer(text.c_str());
   
   switch(tt)
   {
   case litehtml::text_transform_capitalize:
      {
         UTF16 theCh = theBuffer.getChar(0);
         theCh = toupper(theCh);
         theBuffer.setChar(0, theCh);
      }
      break;
   case litehtml::text_transform_uppercase:
      {
         U32 len = theBuffer.length();
         for (U32 i=0; i<len; i++)
         {
            UTF16 theCh = theBuffer.getChar(i);
            theCh = toupper(theCh);
            theBuffer.setChar(i, theCh);
         }
      }
         
      break;
   case litehtml::text_transform_lowercase:
      {
         U32 len = theBuffer.length();
         for (U32 i=0; i<len; i++)
         {
            UTF16 theCh = theBuffer.getChar(i);
            theCh = tolower(theCh);
            theBuffer.setChar(i, theCh);
         }
      }
      break;
   default:
      break;
   }
   
   text = theBuffer.getPtr8();
}

void container_test::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) {}  //: import_css
void container_test::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) {
   // TODO: scissor clip radius
   RectI newClip = RectI(mOffset + Point2I(pos.left(), pos.top()), Point2I(pos.width, pos.height));
   newClip.intersect(mLastClipRect);
   mClipStack.push_back(mLastClipRect);
   mLastClipRect = newClip;
   dglSetClipRect(newClip);
}
void container_test::del_clip() {
   
   if (mClipStack.size() > 0)
   {
      mLastClipRect = mClipStack.back();
      dglSetClipRect(mLastClipRect);
      mClipStack.pop_back();
   }
}

void container_test::get_client_rect(litehtml::position& client) const
{
   RectI bounds = mControl->getBounds();
   client.width = bounds.extent.x;
   client.height = bounds.extent.y;
   client.x = bounds.point.x;
   client.y = bounds.point.y;
}

std::shared_ptr<litehtml::element> container_test::create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc)
{
   // TODO
#if 0
   AbstractClassRep* elKlass = AbstractClassRep::findClassRep(tag_name);
   if (elKlass == NULL)
      return NULL;
   
   AbstractClassRep* controlKlass = GuiControl::getClassRep();
   if (!elKlass->isClass(controlKlass))
      return NULL;
   
   GuiControl* currentNewObject = (GuiControl*)elKlass->create();
   for (auto &attr : attributes)
   {
      // TODO: parse array?
      currentNewObject->setField(attr.first.c_str(), attr.second.c_str());
   }
   
   // Do the constructor parameters.
   char* param = NULL;
   if(!currentNewObject->processArguments(0, &param))
   {
      delete currentNewObject;
      return NULL;
   }
   
   char nameBuf[128];
   dSprintf(nameBuf, sizeof(nameBuf), "html_control_%i", mLastControlId++);
   currentNewObject->setInternalName(nameBuf);
   
   if (!currentNewObject->registerObject())
   {
      return NULL;
   }
   
   currentNewObject->setModStaticFields(true);
   currentNewObject->setModDynamicFields(true);
  
   TrackedHTMLElement* el = new TrackedHTMLElement();
   el->mControl = currentNewObject;
   
   // TODO: control needs a clip state
   mControl->addObject(currentNewObject);
#endif
   return NULL;
}

void container_test::get_media_features(litehtml::media_features& media) const {
  litehtml::position client;
  get_client_rect(client);
  media.type = litehtml::media_type_screen;
  media.width = client.width;
  media.height = client.height;
   
  GuiCanvas* theRoot = mControl->getRoot();
  Point2I theExtent = theRoot->mTargetDrawable.getInfo().textureDimensions;
   
  media.device_width = theExtent.x;
  media.device_height = theExtent.y;
  media.color = 8;
  media.monochrome = 0;
  media.color_index = 256;
  media.resolution = 96;
}

void container_test::get_language(litehtml::tstring& language, litehtml::tstring& culture) const {
  language = _t("en");
  culture = _t("");
}


IMPLEMENT_CONOBJECT(GuiHTMLControl);

GuiHTMLControl::GuiHTMLControl()
{
   mDocContext = NULL;
   mDocContainer = NULL;
}

GuiHTMLControl::~GuiHTMLControl()
{

}

void GuiHTMLControl::initPersistFields()
{
	Parent::initPersistFields();
}

bool GuiHTMLControl::onAdd()
{
   if (Parent::onAdd())
   {
      mDocContext = new context();
      mDocContainer = new container_test();
      mDocContainer->mControl = this;
      mContent = "<html>Body</html>";
      
      FileStream fs;
      if (fs.open("master.css", FileStream::Read))
      {
         char* dat = new char[fs.getStreamSize()+1];
         if (fs.read(fs.getStreamSize(), dat))
         {
            dat[fs.getStreamSize()] = '\0';
            mDocContext->load_master_stylesheet(dat);
         }
         delete[] dat;
      }
      
      return true;
   }
   
   return false;
}

bool GuiHTMLControl::onWake()
{
   if (!Parent::onWake())
      return false;
   
   mDoc = document::createFromString(mContent.c_str(), mDocContainer, mDocContext);
   
   return true;
}

void GuiHTMLControl::setContent(const char* value)
{
   mContent = value;
   mDoc = document::createFromString(mContent.c_str(), mDocContainer, mDocContext);
}

ConsoleMethod(GuiHTMLControl, setContent, void, 3, 3, "")
{
   object->setContent(argv[2]);
}

void GuiHTMLControl::onSleep()
{

}

void GuiHTMLControl::inspectPostApply()
{

}

void GuiHTMLControl::onRender(Point2I offset, const RectI &updateRect)
{
  dglSetBitmapModulation(ColorI(0,0,0,255));
  position pos(0, 0, updateRect.extent.x, updateRect.extent.y);
   //dglDrawText(mProfile->mFont, Point2I(50,50), "TEST");
#if 1
   
   //dglGetMainDevice()->clearRaster(DGLDeviceState::DGL_CLEAR_MASK_RGBA, ColorI(255,0,0,255));
   
   dglPushGroupMarker("HTML Draw");
   mDocContainer->mOffset = offset;
  mDocContainer->mLastClipRect = updateRect;
   
   int theWidth = mDoc->render(getExtent().x);
  mDoc->draw((uint_ptr)0, 0, 0, &pos);
   dglPopGroupMarker();
#endif
}

