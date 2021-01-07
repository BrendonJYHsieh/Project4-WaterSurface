/************************************************************************
     File:        TrainWindow.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						this class defines the window in which the project 
						runs - its the outer windows that contain all of 
						the widgets, including the "TrainView" which has the 
						actual OpenGL window in which the train is drawn

						You might want to modify this class to add new widgets
						for controlling	your train

						This takes care of lots of things - including installing 
						itself into the FlTk "idle" loop so that we get periodic 
						updates (if we're running the train).


     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <FL/fl.h>
#include <FL/Fl_Box.h>

// for using the real time clock
#include <time.h>

#include "TrainWindow.H"
#include "TrainView.H"
#include "CallBacks.H"



//************************************************************************
//
// * Constructor
//========================================================================
TrainWindow::
TrainWindow(const int x, const int y) 
	: Fl_Double_Window(x,y,800,600,"Train and Roller Coaster")
//========================================================================
{
	// make all of the widgets
	begin();	// add to this widget
	{
		int pty=5;			// where the last widgets were drawn

		trainView = new TrainView(5,5,590,590);
		trainView->tw = this;
		trainView->m_pTrack = &m_Track;
		this->resizable(trainView);

		// to make resizing work better, put all the widgets in a group
		widgets = new Fl_Group(600,5,190,590);
		widgets->begin();

		runButton = new Fl_Button(605,pty,60,20,"Run");
		togglify(runButton);

		Fl_Button* fb = new Fl_Button(700,pty,25,20,"@>>");
		fb->callback((Fl_Callback*)forwCB,this);
		Fl_Button* rb = new Fl_Button(670,pty,25,20,"@<<");
		rb->callback((Fl_Callback*)backCB,this);
		
		arcLength = new Fl_Button(730,pty,65,20,"ArcLength");
		togglify(arcLength,1);
  
		pty+=25;
		speed = new Fl_Value_Slider(655,pty,140,20,"speed");
		speed->range(0,10);
		speed->value(2);
		speed->align(FL_ALIGN_LEFT);
		speed->type(FL_HORIZONTAL);

		pty += 25;
		adjust = new Fl_Value_Slider(655, pty, 140, 20, "adjust");
		adjust->range(0.0, 2.0);
		adjust->value(2);
		adjust->align(FL_ALIGN_LEFT);
		adjust->type(FL_HORIZONTAL);

		pty += 30;

		// camera buttons - in a radio button group
		Fl_Group* camGroup = new Fl_Group(600,pty,195,20);
		camGroup->begin();
		worldCam = new Fl_Button(605, pty, 60, 20, "World");
        worldCam->type(FL_RADIO_BUTTON);		// radio button
        worldCam->value(1);			// turned on
        worldCam->selection_color((Fl_Color)3); // yellow when pressed
		worldCam->callback((Fl_Callback*)damageCB,this);
		trainCam = new Fl_Button(670, pty, 60, 20, "Train");
        trainCam->type(FL_RADIO_BUTTON);
        trainCam->value(0);
        trainCam->selection_color((Fl_Color)3);
		trainCam->callback((Fl_Callback*)damageCB,this);
		topCam = new Fl_Button(735, pty, 60, 20, "Top");
        topCam->type(FL_RADIO_BUTTON);
        topCam->value(0);
        topCam->selection_color((Fl_Color)3);
		topCam->callback((Fl_Callback*)damageCB,this);
		camGroup->end();

		pty += 30;

		// browser to select spline types
		// TODO: make sure these choices are the same as what the code supports
		splineBrowser = new Fl_Browser(605,pty,120,75,"Spline Type");
		splineBrowser->type(2);		// select
		splineBrowser->callback((Fl_Callback*)damageCB,this);
		splineBrowser->add("Linear");
		splineBrowser->add("Cardinal Cubic");
		splineBrowser->add("Cubic B-Spline");
		splineBrowser->select(2);

		pty += 100;

		// add and delete points
		Fl_Button* ap = new Fl_Button(605,pty,80,20,"Add Point");
		ap->callback((Fl_Callback*)addPointCB,this);
		Fl_Button* dp = new Fl_Button(690,pty,80,20,"Delete Point");
		dp->callback((Fl_Callback*)deletePointCB,this);

		pty += 25;
		// reset the points
		resetButton = new Fl_Button(735,pty,60,20,"Reset");
		resetButton->callback((Fl_Callback*)resetCB,this);
		Fl_Button* loadb = new Fl_Button(605,pty,60,20,"Load");
		loadb->callback((Fl_Callback*) loadCB, this);
		Fl_Button* saveb = new Fl_Button(670,pty,60,20,"Save");
		saveb->callback((Fl_Callback*) saveCB, this);

		pty += 25;
		// roll the points
		Fl_Button* rx = new Fl_Button(605,pty,30,20,"R+X");
		rx->callback((Fl_Callback*)rpxCB,this);
		Fl_Button* rxp = new Fl_Button(635,pty,30,20,"R-X");
		rxp->callback((Fl_Callback*)rmxCB,this);
		Fl_Button* rz = new Fl_Button(670,pty,30,20,"R+Z");
		rz->callback((Fl_Callback*)rpzCB,this);
		Fl_Button* rzp = new Fl_Button(700,pty,30,20,"R-Z");
		rzp->callback((Fl_Callback*)rmzCB,this);
		pty += 30;

		Fl_Button* add_car = new Fl_Button(605, pty, 60, 20, "Add Car");
		add_car->callback((Fl_Callback*)AddCar, this);
		Fl_Button* delete_car = new Fl_Button(675, pty, 75, 20, "Delete Car");
		delete_car->callback((Fl_Callback*)DeleteCar, this);

		pty += 30;
		// browser to select spline types
		// TODO: make sure these choices are the same as what the code supports
		waveBrowser = new Fl_Browser(605, pty, 120, 75, "Wave Type");
		waveBrowser->type(2);		// select
		waveBrowser->callback((Fl_Callback*)damageCB, this);
		waveBrowser->add("Real Time Reflect");
		waveBrowser->add("Sin Wave");
		waveBrowser->add("HeightMap Wave");
		waveBrowser->add("Interactive Wave");
		waveBrowser->select(1);

		pty+=100;

		WaveFrequency = new Fl_Value_Slider(675, pty, 120, 20, "Freqency");
		WaveFrequency->range(0, 5.0);
		WaveFrequency->value(2.5);
		WaveFrequency->align(FL_ALIGN_LEFT);
		WaveFrequency->type(FL_HORIZONTAL);
	
		pty += 30;

		WaveAmplitude = new Fl_Value_Slider(675, pty, 120, 20, "Amplitude");
		WaveAmplitude->range(0, 0.5);
		WaveAmplitude->value(0.15);
		WaveAmplitude->align(FL_ALIGN_LEFT);
		WaveAmplitude->type(FL_HORIZONTAL);
		Refresh = true;
		
		pty += 30;

		Fl_Button* BDirect = new Fl_Button(605, pty, 70, 20, "Pixelation");
		BDirect->callback((Fl_Callback*)Diret_enable, this);

		Fl_Button* BPoint = new Fl_Button(680, pty, 45, 20, "Toon");
		BPoint->callback((Fl_Callback*)Point_enable, this);

		Fl_Button* BSpot = new Fl_Button(735, pty, 45, 20, "Sketch");
		BSpot->callback((Fl_Callback*)Spot_enable, this);

		pty += 30;

		ReflectBtn = new Fl_Button(605, pty, 70, 20, "Reflect");
		ReflectBtn->callback((Fl_Callback*)Reflect_enable, this);
		RefractBtn = new Fl_Button(685, pty, 70, 20, "Refract");
		RefractBtn->callback((Fl_Callback*)Refract_enable, this);

		pty += 30; 
		buildingCounter = new Fl_Counter(605, pty, 70, 20, "Build");
		buildingCounter->type(2);
		buildingCounter->value(1);
		buildingCounter->step(1);
		buildingCounter->range(1, 5);

		// we need to make a little phantom widget to have things resize correctly
		Fl_Box* resizebox = new Fl_Box(600,595,200,5);
		widgets->resizable(resizebox);

		widgets->end();
	}
	end();	// done adding to this widget

	// set up callback on idle
	Fl::add_idle((void (*)(void*))runButtonCB,this);
	//Fl::add_idle((void (*)(void*))SettingRefresh, this);
	
}

//************************************************************************
//
// * handy utility to make a button into a toggle
//========================================================================
void TrainWindow::
togglify(Fl_Button* b, int val)
//========================================================================
{
	b->type(FL_TOGGLE_BUTTON);		// toggle
	b->value(val);		// turned off
	b->selection_color((Fl_Color)3); // yellow when pressed	
	b->callback((Fl_Callback*)damageCB,this);
}

//************************************************************************
//
// *
//========================================================================
void TrainWindow::
damageMe()
//========================================================================
{
	if (trainView->selectedCube >= ((int)m_Track.points.size()))
		trainView->selectedCube = 0;
	trainView->damage(1);
}

//************************************************************************
//
// * This will get called (approximately) 30 times per second
//   if the run button is pressed
//========================================================================
void TrainWindow::
advanceTrain(float dir)
//========================================================================
{
	//#####################################################################
	// TODO: make this work for your train
	//#####################################################################
	if (trainView->tw->arcLength->value()) {
		m_Track.trainU += (1.5f * speed->value() + trainView->physical * 20) * dir;
		if (m_Track.trainU > trainView->Path_Total) {
			m_Track.trainU -= trainView->Path_Total;
		}
	}
	else {
		if (trainView->tw->arcLength->value() != trainView->s) {
			trainView->t_time = trainView->t_t + trainView->t_i;
		}
		trainView->t_time += (dir * speed->value() / 100);
		m_Track.trainU += 1.0f * speed->value();
	}
	if (trainView->t_time >= trainView->m_pTrack->points.size()) {
		trainView->t_time -= trainView->m_pTrack->points.size();
	}
	trainView->s = trainView->tw->arcLength->value();
}