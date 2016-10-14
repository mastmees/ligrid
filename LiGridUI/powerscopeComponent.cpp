/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 Jul 2009 10:38:55 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.11

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include <math.h>
//[/Headers]

#include "powerscopeComponent.h"

//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
powerscopeComponent::powerscopeComponent ()
    : Component (T("powerScope"))
{

    //[UserPreSize]
    //[/UserPreSize]

    setSize (200, 200);

    //[Constructor] You can add your own custom stuff here..
	SampleData=NULL;
	SampleCount=0;
	needsrepainting=1;
    //[/Constructor]
}

powerscopeComponent::~powerscopeComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void powerscopeComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
	needsrepainting=0;
	g.setColour(Colours::grey);
	g.drawLine(0,100,199,100);
	if (SampleData && SampleCount) {
		int i;
		g.setColour(Colours::blue);
		g.setPixel(0,SampleData[0]);
		for (i=1;i<SampleCount;i++) {
			g.drawLine((i-1),SampleData[(i-1)],i,SampleData[i]);
		}
	}
	
    //[/UserPaint]
}

void powerscopeComponent::resized()
{
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void powerscopeComponent::SetData(int nsamples,int *data)
{
	int *newsamples=new int[nsamples],*osamples,i,db;
	double p;
	for (i=0;i<nsamples*2;i+=2) {
		p=sqrt(data[i]*data[i]+data[i+1]*data[i+1]);
		if (!p)
			db=100;
		else
			db=0-20.0*log10(p/32768.0);
		if (db<0) db=0;
		newsamples[i/2]=db;
	}
	SampleCount=nsamples;
	osamples=SampleData;
	SampleData=newsamples;
	if (osamples)
		delete osamples;
	needsrepainting++;
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="powerscopeComponent" componentName="powerScope"
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="200" initialHeight="200">
  <BACKGROUND backgroundColour="ffffffff"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
