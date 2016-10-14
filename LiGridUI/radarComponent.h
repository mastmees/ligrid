/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  16 Jul 2009 1:33:53 am

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.11

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_RADARCOMPONENT_RADARCOMPONENT_5D5F9819__
#define __JUCER_HEADER_RADARCOMPONENT_RADARCOMPONENT_5D5F9819__

//[Headers]     -- You can add your own extra header files here --
#include "../juce/juce.h"
#include "../ipnetwork.hpp"

class Station : public Thread
	{
	public:
		CTCPConnection connection;
		enum { LINKDOWN,LINKUP,TERMINATING,STOPPED,DELETED };
		IPADDR adr;
		double latitude,longitude;
		char *name;
		int status;
		
		class Station* nextstation;
		
		Station(IPADDR adr,double latitude,double longitude,char *name) : Thread (T("StationThead"))
		{
			status=LINKDOWN;
			this->adr=adr;
			connection.remoteaddr=adr;
			this->latitude=latitude;
			this->longitude=longitude;
			this->name=new char[strlen(name)+1];
			strcpy(this->name,name);
		}
		
		~Station()
		{
			status=TERMINATING;
			if (connection.GetSd()!=-1)
				connection.Close();
			if (name)
				delete name;
			status=DELETED;
		}
		
		void run()
		{
			while (!threadShouldExit() && connection.GetSd()!=-1) {
				if (connection.DataReady()) {
					char s[4096];
					if (connection.ReadLine(s,sizeof s)) {
					}
				}
			}
			status=TERMINATING;
			connection.Close();
			status=STOPPED;
		}
		
		void Stop()
		{
		}
		
		void Start()
		{
		}
	};

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class radarComponent  : public Component
{
public:
    //==============================================================================
    radarComponent ();
    ~radarComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	void AddStation(char *stationinfo);
	void ClearStations();
	CriticalSection mutex;
	Station *Stations;
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();

    // Binary resources:
    static const char* equirectangularprojection_jpg;
    static const int equirectangularprojection_jpgSize;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    Image* internalCachedImage1;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    radarComponent (const radarComponent&);
    const radarComponent& operator= (const radarComponent&);
};


#endif   // __JUCER_HEADER_RADARCOMPONENT_RADARCOMPONENT_5D5F9819__
