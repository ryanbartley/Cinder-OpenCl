#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "MetaBallsController.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MetaBallsApp : public App {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();
	
	MetaBallsController mMetaballsController;
};

void MetaBallsApp::setup()
{
	mMetaballsController.setup();
}

void MetaBallsApp::mouseDown( MouseEvent event )
{
	mMetaballsController.mouseDown( event );
}

void MetaBallsApp::mouseDrag( MouseEvent event )
{
	mMetaballsController.mouseDrag( event );
}

void MetaBallsApp::update()
{
	mMetaballsController.update();
}

void MetaBallsApp::draw()
{
	mMetaballsController.draw();
	getWindow()->setTitle( to_string( getAverageFps() ) );
}

CINDER_APP( MetaBallsApp, RendererGl, []( App::Settings *settings ) { settings->setConsoleWindowEnabled();  } )
