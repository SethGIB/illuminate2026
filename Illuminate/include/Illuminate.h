#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

enum class IlluminateStatus {
	PENDING,
	REJECTED,
	ACCEPTED
};

struct IlluminateArtist {
public:
	string Name;
	vector<string> Genres;
	vector<string> Projects;
	IlluminateStatus Status;
};

class IlluminateApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;

	IlluminateArtist *Artist;
};