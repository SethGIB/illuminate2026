#include "Illuminate.h"

void IlluminateApp::setup()
{
	Artist = new IlluminateArtist();
	Artist->Name = "OutofYerHed";
	Artist->Genres =
	{
		"Creative Tech",
		"Digital",
		"Interactive",
		"Realtime"
	};
	Artist->Projects =
	{
		"Living Color",
		"Tag, You're It!",
		"Collab w/ @de.izzy.art"
	};
	Artist->Status = IlluminateStatus::ACCEPTED;
}

void IlluminateApp::mouseDown( MouseEvent event )
{
}

void IlluminateApp::update()
{
}

void IlluminateApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( IlluminateApp, RendererGl )
